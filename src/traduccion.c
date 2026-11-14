#include "traduccion.h"

#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <windows.h>

static int modo_traduccion;
static int usa_f32;
static int usa_idiv;
static Expresion convertir_tipo(Expresion valor, Tipo destino);

char *copiar_lexema(const char *texto, size_t longitud)
{
    char *copia;

    if (longitud == SIZE_MAX) {
        return NULL;
    }
    copia = malloc(longitud + 1);
    if (copia != NULL) {
        memcpy(copia, texto, longitud);
        copia[longitud] = '\0';
    }
    return copia;
}

Expresion expresion_desde_lexema(char *texto, Tipo tipo)
{
    Expresion resultado = {texto, tipo, PREC_ATOMO};
    return resultado;
}

static Tipo tipo_comun(Tipo izquierda, Tipo derecha)
{
    /* Sin tabla de simbolos todavia no se conoce el tipo de un ID. */
    if (izquierda == T_DESCONOCIDO || derecha == T_DESCONOCIDO) {
        return T_DESCONOCIDO;
    }
    if (izquierda == T_DOUBLE || derecha == T_DOUBLE) {
        return T_DOUBLE;
    }
    if (izquierda == T_FLOAT || derecha == T_FLOAT) {
        return T_FLOAT;
    }
    return T_INT;
}

static Expresion binaria_texto(Expresion izquierda, const char *operador,
                       Expresion derecha, Precedencia precedencia)
{
    Expresion resultado = {NULL, tipo_comun(izquierda.tipo, derecha.tipo),
                           precedencia};
    int parentesis_izquierda = izquierda.precedencia < precedencia;
    /* El lado derecho conserva la agrupacion incluso en sumas de reales. */
    int parentesis_derecha = derecha.precedencia <= precedencia;
    int longitud;

    if (izquierda.texto == NULL || derecha.texto == NULL) {
        goto liberar;
    }
    longitud = snprintf(NULL, 0, "%s%s%s %s %s%s%s",
                        parentesis_izquierda ? "(" : "", izquierda.texto,
                        parentesis_izquierda ? ")" : "", operador,
                        parentesis_derecha ? "(" : "", derecha.texto,
                        parentesis_derecha ? ")" : "");
    if (longitud < 0) {
        goto liberar;
    }
    resultado.texto = malloc((size_t)longitud + 1);
    if (resultado.texto != NULL) {
        snprintf(resultado.texto, (size_t)longitud + 1, "%s%s%s %s %s%s%s",
                 parentesis_izquierda ? "(" : "", izquierda.texto,
                 parentesis_izquierda ? ")" : "", operador,
                 parentesis_derecha ? "(" : "", derecha.texto,
                 parentesis_derecha ? ")" : "");
    }

liberar:
    liberar_expresion(&izquierda);
    liberar_expresion(&derecha);
    return resultado;
}

Expresion crear_negacion(Expresion operando)
{
    Expresion resultado = {NULL, operando.tipo, PREC_UNARIO};
    int parentesis = operando.precedencia <= PREC_UNARIO;
    int longitud;

    if (operando.texto == NULL) {
        return resultado;
    }
    longitud = snprintf(NULL, 0, "-%s%s%s", parentesis ? "(" : "",
                        operando.texto, parentesis ? ")" : "");
    if (longitud >= 0) {
        resultado.texto = malloc((size_t)longitud + 1);
        if (resultado.texto != NULL) {
            snprintf(resultado.texto, (size_t)longitud + 1, "-%s%s%s",
                     parentesis ? "(" : "", operando.texto,
                     parentesis ? ")" : "");
        }
    }
    liberar_expresion(&operando);
    return resultado;
}

void liberar_expresion(Expresion *expresion)
{
    free(expresion->texto);
    expresion->texto = NULL;
}

/* Auxiliares de texto. La propiedad de los fragmentos pasa al resultado. */
static char *formatear(const char *formato, ...)
{
    va_list argumentos, copia;
    int longitud;
    char *texto;
    va_start(argumentos, formato);
    va_copy(copia, argumentos);
    longitud = vsnprintf(NULL, 0, formato, copia);
    va_end(copia);
    texto = longitud < 0 ? NULL : malloc((size_t)longitud + 1);
    if (texto == NULL) {
        fputs("Error: memoria insuficiente para construir el codigo.\n", stderr);
        exit(EXIT_FAILURE);
    }
    vsnprintf(texto, (size_t)longitud + 1, formato, argumentos);
    va_end(argumentos);
    return texto;
}

char *texto_vacio(void)
{
    return formatear("%s", "");
}

char *unir_textos(char *primero, char *segundo)
{
    char *resultado = formatear("%s%s", primero, segundo);
    free(primero);
    free(segundo);
    return resultado;
}

typedef struct {
    char *nombre_c;
    char *nombre_python;
    Tipo tipo;
    int ambito;
    int activa;
    int inicializada;
} Simbolo;

static Simbolo simbolos[MAX_SIMBOLOS];
static int cantidad_simbolos;
static int ambito_actual;
static int alcanzable;
int errores_semanticos;

static void error_semantico(int linea, int columna, const char *formato, ...)
{
    va_list argumentos;
    ++errores_semanticos;
    fprintf(stderr, "Error semantico en la linea %d, columna %d: ", linea, columna);
    va_start(argumentos, formato);
    vfprintf(stderr, formato, argumentos);
    va_end(argumentos);
    fputc('\n', stderr);
}

void iniciar_traduccion(int habilitada)
{
    modo_traduccion = habilitada;
    usa_f32 = usa_idiv = errores_semanticos = 0;
    cantidad_simbolos = ambito_actual = 0;
    alcanzable = 1;
}

void finalizar_traduccion(void)
{
    int i;
    for (i = 0; i < cantidad_simbolos; ++i) {
        free(simbolos[i].nombre_c);
        free(simbolos[i].nombre_python);
    }
    cantidad_simbolos = 0;
}

void entrar_ambito(void)
{
    ++ambito_actual;
}

void salir_ambito(void)
{
    int i;
    for (i = 0; i < cantidad_simbolos; ++i) {
        if (simbolos[i].ambito == ambito_actual) {
            simbolos[i].activa = 0;
        }
    }
    --ambito_actual;
}

static int nombre_reservado(const char *nombre)
{
    /* Solo se reservan palabras Python y nombres que utiliza el generador. */
    static const char *reservados[] = {
        "False", "None", "True", "and", "as", "assert", "async", "await",
        "break", "class", "continue", "def", "del", "elif", "else", "except",
        "finally", "for", "from", "global", "if", "import", "in", "is",
        "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
        "while", "with", "yield", "print", "int", "float", "abs", "SystemExit",
        "__name__", NULL
    };
    int i;
    if (strncmp(nombre, "_c_", 3) == 0) {
        return 1;
    }
    for (i = 0; reservados[i] != NULL; ++i) {
        if (strcmp(nombre, reservados[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int declarar_simbolo(const char *nombre, Tipo tipo, int linea, int columna)
{
    int i, conflicto = nombre_reservado(nombre);
    Simbolo *simbolo;
    if (!modo_traduccion) {
        return -1;
    }
    for (i = 0; i < cantidad_simbolos; ++i) {
        if (simbolos[i].activa && simbolos[i].ambito == ambito_actual &&
            strcmp(nombre, simbolos[i].nombre_c) == 0) {
            error_semantico(linea, columna, "la variable '%s' ya esta declarada en este bloque", nombre);
            return -1;
        }
        if (strcmp(nombre, simbolos[i].nombre_python) == 0) {
            conflicto = 1;
        }
    }
    if (cantidad_simbolos == MAX_SIMBOLOS) {
        error_semantico(linea, columna, "se excedio el limite de %d declaraciones", MAX_SIMBOLOS);
        return -1;
    }
    simbolo = &simbolos[cantidad_simbolos];
    simbolo->nombre_c = formatear("%s", nombre);
    simbolo->nombre_python = conflicto ? formatear("_c_var_%d", cantidad_simbolos + 1)
                                        : formatear("%s", nombre);
    simbolo->tipo = tipo;
    simbolo->ambito = ambito_actual;
    simbolo->activa = 1;
    simbolo->inicializada = 0;
    return cantidad_simbolos++;
}

int buscar_simbolo(const char *nombre, int linea, int columna)
{
    int i;
    if (!modo_traduccion) {
        return -1;
    }
    for (i = cantidad_simbolos - 1; i >= 0; --i) {
        if (simbolos[i].activa && strcmp(nombre, simbolos[i].nombre_c) == 0) {
            return i;
        }
    }
    error_semantico(linea, columna, "variable '%s' no declarada en este ambito", nombre);
    return -1;
}

Expresion referencia_variable(char *nombre, int linea, int columna)
{
    int indice;
    Expresion resultado;
    if (!modo_traduccion) {
        return expresion_desde_lexema(nombre, T_DESCONOCIDO);
    }
    indice = buscar_simbolo(nombre, linea, columna);
    if (indice < 0) {
        free(nombre);
        return expresion_desde_lexema(formatear("0"), T_DESCONOCIDO);
    }
    if (alcanzable && !simbolos[indice].inicializada) {
        error_semantico(linea, columna, "variable '%s' usada sin inicializacion garantizada", nombre);
    }
    resultado = expresion_desde_lexema(formatear("%s", simbolos[indice].nombre_python), simbolos[indice].tipo);
    free(nombre);
    return resultado;
}

Expresion literal_numerico(char *texto, Tipo tipo, int linea, int columna)
{
    char *fin;
    if (modo_traduccion) {
        errno = 0;
        if (tipo == T_INT) {
            long long valor = strtoll(texto, &fin, 0);
            if (errno || *fin || valor > INT_MAX) {
                error_semantico(linea, columna, "literal entero fuera del perfil int: '%s'", texto);
            } else {
                char *decimal = formatear("%lld", valor);
                free(texto);
                texto = decimal;
            }
        } else {
            double valor = strtod(texto, &fin);
            if (errno || *fin || !isfinite(valor)) {
                error_semantico(linea, columna, "literal real fuera de rango: '%s'", texto);
            }
        }
    }
    return expresion_desde_lexema(texto, tipo);
}

static Expresion convertir_tipo(Expresion valor, Tipo destino)
{
    char *nuevo;
    if (valor.tipo == destino || valor.tipo == T_DESCONOCIDO) {
        return valor;
    }
    if (destino == T_FLOAT) {
        usa_f32 = 1;
        nuevo = formatear("_c_f32(%s)", valor.texto);
    } else if (destino == T_INT) {
        nuevo = formatear("int(%s)", valor.texto);
    } else if (valor.tipo == T_INT) {
        nuevo = formatear("float(%s)", valor.texto);
    } else {
        valor.tipo = destino;
        return valor;
    }
    liberar_expresion(&valor);
    return expresion_desde_lexema(nuevo, destino);
}

Expresion crear_binaria(Expresion izquierda, const char *operador,
                       Expresion derecha, Precedencia precedencia)
{
    Tipo tipo = tipo_comun(izquierda.tipo, derecha.tipo);
    Expresion resultado;
    if (!modo_traduccion) {
        return binaria_texto(izquierda, operador, derecha, precedencia);
    }
    izquierda = convertir_tipo(izquierda, tipo);
    derecha = convertir_tipo(derecha, tipo);
    if (tipo == T_INT && strcmp(operador, "/") == 0) {
        usa_idiv = 1;
        resultado = expresion_desde_lexema(
            formatear("_c_idiv(%s, %s)", izquierda.texto, derecha.texto), T_INT);
        liberar_expresion(&izquierda);
        liberar_expresion(&derecha);
    } else {
        resultado = binaria_texto(izquierda, operador, derecha, precedencia);
        if (resultado.texto != NULL && tipo == T_FLOAT) {
            char *redondeada = formatear("_c_f32(%s)", resultado.texto);
            usa_f32 = 1;
            liberar_expresion(&resultado);
            resultado = expresion_desde_lexema(redondeada, T_FLOAT);
        }
    }
    return resultado;
}

Expresion crear_comparacion(Expresion izquierda, const char *operador,
                           Expresion derecha)
{
    if (modo_traduccion) {
        Tipo tipo = tipo_comun(izquierda.tipo, derecha.tipo);
        izquierda = convertir_tipo(izquierda, tipo);
        derecha = convertir_tipo(derecha, tipo);
    }
    return binaria_texto(izquierda, operador, derecha, PREC_RELACION);
}

char *generar_asignacion(int indice, Expresion valor)
{
    char *codigo = texto_vacio();
    if (modo_traduccion && indice >= 0) {
        valor = convertir_tipo(valor, simbolos[indice].tipo);
        free(codigo);
        codigo = formatear("%s = %s\n", simbolos[indice].nombre_python, valor.texto);
        simbolos[indice].inicializada = 1;
    }
    liberar_expresion(&valor);
    return codigo;
}

char *generar_return(Expresion valor)
{
    char *codigo = texto_vacio();
    if (modo_traduccion) {
        valor = convertir_tipo(valor, T_INT);
        free(codigo);
        codigo = formatear("return %s\n", valor.texto);
        alcanzable = 0;
    }
    liberar_expresion(&valor);
    return codigo;
}

Argumentos primer_argumento(Expresion valor)
{
    Argumentos argumentos = {NULL, 0};
    return agregar_argumento(argumentos, valor);
}

Argumentos agregar_argumento(Argumentos argumentos, Expresion valor)
{
    Expresion *elementos = realloc(argumentos.elementos,
                                   (argumentos.cantidad + 1) * sizeof *elementos);
    if (elementos == NULL) {
        fputs("Error: memoria insuficiente para los argumentos.\n", stderr);
        exit(EXIT_FAILURE);
    }
    argumentos.elementos = elementos;
    argumentos.elementos[argumentos.cantidad++] = valor;
    return argumentos;
}

void liberar_argumentos(Argumentos *argumentos)
{
    size_t i;
    for (i = 0; i < argumentos->cantidad; ++i) {
        liberar_expresion(&argumentos->elementos[i]);
    }
    free(argumentos->elementos);
    argumentos->elementos = NULL;
    argumentos->cantidad = 0;
}

/* Se decodifican escapes C y se vuelve a citar la cadena para Python. */
static char *decodificar_cadena(const char *cadena, int linea, int columna)
{
    size_t i, j = 0, longitud = strlen(cadena);
    char *resultado = formatear("%s", cadena);
    for (i = 1; i + 1 < longitud; ++i) {
        char caracter = cadena[i];
        if (caracter == '\\') {
            caracter = cadena[++i];
            switch (caracter) {
                case 'n': caracter = '\n'; break;
                case 't': caracter = '\t'; break;
                case 'r': caracter = '\r'; break;
                case '\\': case '"': break;
                default:
                    error_semantico(linea, columna, "escape \\%c no admitido", caracter);
                    break;
            }
        }
        resultado[j++] = caracter;
    }
    resultado[j] = '\0';
    return resultado;
}

static char *citar_python(const char *cadena)
{
    size_t i, j = 0, longitud = strlen(cadena);
    char *resultado = malloc(longitud * 4 + 3);
    if (resultado == NULL) {
        fputs("Error: memoria insuficiente para una cadena.\n", stderr);
        exit(EXIT_FAILURE);
    }
    resultado[j++] = '"';
    for (i = 0; i < longitud; ++i) {
        unsigned char c = (unsigned char)cadena[i];
        if (c == '\\' || c == '"') {
            resultado[j++] = '\\'; resultado[j++] = (char)c;
        } else if (c == '\n' || c == '\r' || c == '\t') {
            resultado[j++] = '\\';
            resultado[j++] = c == '\n' ? 'n' : c == '\r' ? 'r' : 't';
        } else if (c < 32 || c == 127) {
            snprintf(resultado + j, 5, "\\x%02x", (unsigned int)c);
            j += 4;
        } else {
            resultado[j++] = (char)c;
        }
    }
    resultado[j++] = '"'; resultado[j] = '\0';
    return resultado;
}

char *generar_printf(char *cadena, Argumentos argumentos, int linea, int columna)
{
    char *codigo = texto_vacio();
    if (modo_traduccion) {
        char *contenido = decodificar_cadena(cadena, linea, columna);
        char *literal, *lista = texto_vacio();
        size_t i, usados = 0;
        int tiene_formato = 0;
        for (i = 0; contenido[i]; ++i) {
            int requiere_real = 0;
            if (contenido[i] != '%') { continue; }
            tiene_formato = 1;
            ++i;
            if (contenido[i] == '%') { continue; }
            if (contenido[i] == '.') {
                ++i;
                if (!isdigit((unsigned char)contenido[i])) {
                    error_semantico(linea, columna, "precision de printf no admitida");
                    break;
                }
                while (isdigit((unsigned char)contenido[i])) { ++i; }
                requiere_real = 1;
            }
            if (contenido[i] == 'f') {
                requiere_real = 1;
            } else if (contenido[i] != 'd' || requiere_real) {
                error_semantico(linea, columna, "formato printf no admitido (solo %%d, %%f, %%.Nf y %%%% )");
                break;
            }
            if (usados < argumentos.cantidad) {
                Tipo tipo = argumentos.elementos[usados].tipo;
                if (tipo != T_DESCONOCIDO &&
                    ((requiere_real && tipo == T_INT) || (!requiere_real && tipo != T_INT))) {
                    error_semantico(linea, columna, "tipo incompatible en el argumento %zu de printf", usados + 1);
                }
            }
            ++usados;
        }
        if (usados != argumentos.cantidad) {
            error_semantico(linea, columna, "printf requiere %zu argumentos y recibio %zu", usados, argumentos.cantidad);
        }
        literal = citar_python(contenido);
        for (i = 0; i < argumentos.cantidad; ++i) {
            lista = unir_textos(lista, formatear("%s%s", i ? ", " : "", argumentos.elementos[i].texto));
        }
        free(codigo);
        if (argumentos.cantidad) {
            codigo = formatear("print(%s %% (%s,), end=\"\")\n", literal, lista);
        } else if (tiene_formato) {
            codigo = formatear("print(%s %% (), end=\"\")\n", literal);
        } else {
            codigo = formatear("print(%s, end=\"\")\n", literal);
        }
        free(contenido); free(literal); free(lista);
    }
    free(cadena);
    liberar_argumentos(&argumentos);
    return codigo;
}

static Estado guardar_estado(void)
{
    Estado estado = {{0}, cantidad_simbolos, alcanzable};
    int i;
    for (i = 0; i < cantidad_simbolos; ++i) {
        estado.inicializadas[i] = (unsigned char)simbolos[i].inicializada;
    }
    return estado;
}

static void restaurar_estado(Estado estado)
{
    int i;
    alcanzable = estado.alcanzable;
    for (i = 0; i < cantidad_simbolos; ++i) {
        simbolos[i].inicializada = i < estado.cantidad ? estado.inicializadas[i] : 0;
    }
}

static char *indentar_bloque(const char *bloque)
{
    size_t i, j = 0, lineas = 0, longitud = strlen(bloque);
    char *resultado;
    if (!longitud) { return formatear("    pass\n"); }
    for (i = 0; i < longitud; ++i) { if (bloque[i] == '\n') { ++lineas; } }
    resultado = malloc(longitud + 4 * (lineas + 1) + 1);
    if (resultado == NULL) {
        fputs("Error: memoria insuficiente para indentar un bloque.\n", stderr);
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < longitud; ++i) {
        if (i == 0 || bloque[i - 1] == '\n') {
            memcpy(resultado + j, "    ", 4); j += 4;
        }
        resultado[j++] = bloque[i];
    }
    resultado[j] = '\0';
    return resultado;
}

ContextoIf *iniciar_if(Expresion condicion)
{
    ContextoIf *contexto = malloc(sizeof *contexto);
    if (contexto == NULL) {
        fputs("Error: memoria insuficiente para un condicional.\n", stderr);
        exit(EXIT_FAILURE);
    }
    contexto->condicion = condicion.texto;
    contexto->entrada = guardar_estado();
    contexto->primera_rama = contexto->entrada;
    return contexto;
}

void preparar_else(ContextoIf *contexto)
{
    contexto->primera_rama = guardar_estado();
    restaurar_estado(contexto->entrada);
}

void liberar_contexto_if(ContextoIf *contexto)
{
    if (contexto != NULL) { free(contexto->condicion); free(contexto); }
}

char *terminar_if(ContextoIf *contexto, char *bloque, char *alternativa)
{
    Estado segunda_rama = guardar_estado();
    Estado primera = contexto->primera_rama;
    char *indentado = indentar_bloque(bloque);
    char *codigo = formatear("if %s:\n%s%s", contexto->condicion, indentado, alternativa);
    int i;
    alcanzable = primera.alcanzable || segunda_rama.alcanzable;
    for (i = 0; i < contexto->entrada.cantidad; ++i) {
        simbolos[i].inicializada =
            (!primera.alcanzable || primera.inicializadas[i]) &&
            (!segunda_rama.alcanzable || segunda_rama.inicializadas[i]);
    }
    free(indentado); free(bloque); free(alternativa);
    liberar_contexto_if(contexto);
    return codigo;
}

char *generar_else(char *bloque)
{
    char *indentado = indentar_bloque(bloque);
    char *codigo = formatear("else:\n%s", indentado);
    free(indentado); free(bloque);
    return codigo;
}

char *generar_elif(char *sentencia_if)
{
    /* La produccion ELSE sentencia_if aporta un if ya construido por Bison. */
    char *codigo = formatear("el%s", sentencia_if);
    free(sentencia_if);
    return codigo;
}

static int misma_entrada(const char *entrada, const char *salida)
{
    HANDLE a, b;
    BY_HANDLE_FILE_INFORMATION info_a, info_b;
    int iguales = 0;
    char *ruta_a = _fullpath(NULL, entrada, 0);
    char *ruta_b = _fullpath(NULL, salida, 0);
    if (ruta_a != NULL && ruta_b != NULL) { iguales = _stricmp(ruta_a, ruta_b) == 0; }
    free(ruta_a); free(ruta_b);
    if (iguales) { return 1; }
    a = CreateFileA(entrada, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    b = CreateFileA(salida, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (a != INVALID_HANDLE_VALUE && b != INVALID_HANDLE_VALUE &&
        GetFileInformationByHandle(a, &info_a) && GetFileInformationByHandle(b, &info_b)) {
        iguales = info_a.dwVolumeSerialNumber == info_b.dwVolumeSerialNumber &&
                  info_a.nFileIndexHigh == info_b.nFileIndexHigh &&
                  info_a.nFileIndexLow == info_b.nFileIndexLow;
    }
    if (a != INVALID_HANDLE_VALUE) { CloseHandle(a); }
    if (b != INVALID_HANDLE_VALUE) { CloseHandle(b); }
    return iguales;
}

int publicar_programa(const char *entrada, const char *salida, const char *cuerpo)
{
    FILE *archivo_salida;
    char *temporal = NULL;
    char *indentado;
    int descriptor = -1, intento, correcto, cierre;
    if (misma_entrada(entrada, salida)) {
        fputs("Error: entrada y salida identifican el mismo archivo.\n", stderr);
        return 0;
    }
    indentado = indentar_bloque(cuerpo);
    for (intento = 0; intento < 100; ++intento) {
        free(temporal);
        temporal = formatear("%s.tmp.%lu.%d", salida, (unsigned long)GetCurrentProcessId(), intento);
        descriptor = _open(temporal, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _S_IREAD | _S_IWRITE);
        if (descriptor >= 0 || errno != EEXIST) { break; }
    }
    if (descriptor < 0) {
        perror("No se pudo crear el archivo temporal de salida");
        free(temporal); free(indentado);
        return 0;
    }
    archivo_salida = _fdopen(descriptor, "wb");
    if (archivo_salida == NULL) {
        perror("No se pudo abrir el flujo de salida");
        _close(descriptor); DeleteFileA(temporal);
        free(temporal); free(indentado);
        return 0;
    }
    if (usa_f32) {
        fprintf(archivo_salida,
                "from struct import pack as _c_pack, unpack as _c_unpack\n\n\n"
                "def _c_f32(valor):\n"
                "    return _c_unpack(\"=f\", _c_pack(\"=f\", valor))[0]\n\n\n");
    }
    if (usa_idiv) {
        fprintf(archivo_salida,
                "def _c_idiv(a, b):\n"
                "    cociente = abs(a) // abs(b)\n"
                "    return -cociente if (a < 0) != (b < 0) else cociente\n\n\n");
    }
    fprintf(archivo_salida, "def main():\n%s\n\n"
            "if __name__ == \"__main__\":\n"
            "    raise SystemExit(main())\n", indentado);
    correcto = fflush(archivo_salida) == 0 && !ferror(archivo_salida);
    cierre = fclose(archivo_salida);
    correcto = correcto && cierre == 0;
    if (correcto) {
        correcto = MoveFileExA(temporal, salida, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
        if (!correcto) { fprintf(stderr, "No se pudo publicar la salida (Windows %lu).\n", (unsigned long)GetLastError()); }
    } else {
        fputs("Error al escribir o cerrar el archivo de salida.\n", stderr);
    }
    if (!correcto) { DeleteFileA(temporal); }
    free(temporal); free(indentado);
    return correcto;
}
