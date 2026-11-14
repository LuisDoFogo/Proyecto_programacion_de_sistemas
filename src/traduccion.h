#ifndef TRADUCCION_H
#define TRADUCCION_H

#include <stddef.h>

/* DOUBLE describe los literales reales; no agrega declaraciones a la gramatica. */
typedef enum {
    T_DESCONOCIDO,
    T_INT,
    T_FLOAT,
    T_DOUBLE
} Tipo;

typedef enum {
    PREC_RELACION = 5,
    PREC_SUMA = 10,
    PREC_PRODUCTO = 20,
    PREC_UNARIO = 30,
    PREC_ATOMO = 40
} Precedencia;

/* Conservamos la expresion escrita, sin calcular su resultado. */
typedef struct {
    char *texto;
    Tipo tipo;
    Precedencia precedencia;
} Expresion;

char *copiar_lexema(const char *texto, size_t longitud);
Expresion expresion_desde_lexema(char *texto, Tipo tipo);
Expresion crear_binaria(Expresion izquierda, const char *operador,
                       Expresion derecha, Precedencia precedencia);
Expresion crear_negacion(Expresion operando);
void liberar_expresion(Expresion *expresion);

#define MAX_SIMBOLOS 100

typedef struct {
    Expresion *elementos;
    size_t cantidad;
} Argumentos;

/* Cada if guarda el estado de las variables antes y despues de su rama. */
typedef struct {
    unsigned char inicializadas[MAX_SIMBOLOS];
    int cantidad;
    int alcanzable;
} Estado;

typedef struct {
    char *condicion;
    Estado entrada;
    Estado primera_rama;
} ContextoIf;

extern int errores_semanticos;

void iniciar_traduccion(int habilitada);
void finalizar_traduccion(void);
void entrar_ambito(void);
void salir_ambito(void);
int declarar_simbolo(const char *nombre, Tipo tipo, int linea, int columna);
int buscar_simbolo(const char *nombre, int linea, int columna);
Expresion referencia_variable(char *nombre, int linea, int columna);
Expresion literal_numerico(char *texto, Tipo tipo, int linea, int columna);
Expresion crear_comparacion(Expresion izquierda, const char *operador,
                           Expresion derecha);
char *generar_asignacion(int indice, Expresion valor);
char *generar_return(Expresion valor);
Argumentos primer_argumento(Expresion valor);
Argumentos agregar_argumento(Argumentos argumentos, Expresion valor);
void liberar_argumentos(Argumentos *argumentos);
char *generar_printf(char *cadena, Argumentos argumentos, int linea, int columna);
char *texto_vacio(void);
char *unir_textos(char *primero, char *segundo);
ContextoIf *iniciar_if(Expresion condicion);
void preparar_else(ContextoIf *contexto);
char *terminar_if(ContextoIf *contexto, char *bloque, char *alternativa);
void liberar_contexto_if(ContextoIf *contexto);
char *generar_else(char *bloque);
char *generar_elif(char *sentencia_if);
int publicar_programa(const char *entrada, const char *salida, const char *cuerpo);

/* Los constructores reciben la propiedad de los lexemas y operandos. */
/* Ante falta de memoria devuelven una expresion con texto == NULL. */

#endif
