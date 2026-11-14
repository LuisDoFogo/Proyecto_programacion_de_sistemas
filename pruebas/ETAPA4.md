# Informe de la etapa 4

> Copia pública anonimizada: las rutas locales se sustituyeron por
> marcadores y las invocaciones personales de Python por `python`.
> Los resultados de las pruebas no se modificaron. Se conserva una
> copia íntegra del informe original fuera del repositorio.

Se implementaron el análisis semántico y la generación automática de Python
mediante Flex, Bison y C. Los tres archivos definitivos de `salida_python/`
no se generaron: corresponden a la etapa 5.

## Protección y archivos

Antes de editar se revisaron `git status --short` y los cambios existentes.
Se conservaron las modificaciones aprobadas de la etapa 3 y los materiales
del profesor que ya estaban sin seguimiento en Git.

El respaldo previo está fuera del proyecto:

```text
%TEMP%\ProyectoProgramacionDeSistemas-etapa4-20261113-222811
```

Contiene las fuentes y los artefactos que sobrescribe la compilación, el
estado inicial de Git y un manifiesto SHA-256. Se comprobaron los 28 archivos
protegidos al iniciar esta etapa: PDF, ejemplos originales, material docente
y pruebas/documentación anteriores. Ninguno cambió ni desapareció.

Archivos modificados en esta etapa:

| Archivo | Responsabilidad |
|---|---|
| `src/parser.y` | Acciones semánticas completas y selección del modo de ejecución. |
| `src/traduccion.h` | Tipos compartidos y declaraciones de funciones auxiliares. |
| `src/traduccion.c` | Símbolos, expresiones tipadas, flujo de inicialización, generación y publicación. |

Archivos creados:

| Archivo | Finalidad |
|---|---|
| `pruebas/probar_etapa4.py` | Pruebas semánticas, comparación C/Python e integridad de archivos. |
| `pruebas/ETAPA4.md` | Este informe. |

No se cambiaron el lexer, el script de compilación ni las pruebas anteriores
respecto de la etapa 3. La compilación regeneró los archivos intermedios y el
ejecutable que ya estaban excluidos de Git. No se realizaron commits ni push.

## Relación con las clases

Se mantienen los mecanismos de los cinco ejemplos del profesor:

- Los ejemplos 01 y 02 aportan `yytext`, `yylval`, `%union`, `%token`, `%type`,
  los atributos `$1`, `$2`, `$3` y `$$`, y la idea de tabla de símbolos.
- Los ejemplos 03 y 04 aportan las acciones de generación, `yyin`,
  `FILE *archivo_salida` y `fprintf()`.
- El ejemplo 05 aporta comprobaciones de declaración y asignación. Aquí se
  guardan tipos y nombres en lugar de calcular valores durante el análisis.
- Se conserva la comunicación léxico/sintáctico y el proceso Flex + Bison +
  GCC explicado en el PDF. No hay un traductor implementado en Python.

Python se usa en las pruebas para validar y ejecutar el código producido,
no para decidir cómo traducir el archivo C.

## Modalidades

```bash
# Conserva el análisis léxico y sintáctico original.
./compilador.exe ejemplos/ejemplo1.c

# Activa comprobaciones semánticas y generación de Python.
./compilador.exe entrada.c salida.py

# Se conserva la entrada estándar en el modo de análisis.
./compilador.exe < pruebas/valido_comentarios.c
```

Con un argumento se conserva el alcance original del análisis; por ejemplo,
no se exige que una variable esté declarada. Con dos argumentos de archivo
se activa el análisis semántico obligatorio para traducir. Esto permite
mantener sin cambios las 33 regresiones de la etapa 3.

## Tabla de símbolos e inicialización

La tabla es un arreglo de hasta 100 declaraciones, con búsqueda lineal.
Cada entrada contiene nombre C, nombre Python, tipo, ámbito, estado activo
y estado de inicialización. Los nombres se conservan salvo conflicto con
palabras reservadas de Python, funciones utilizadas por el código generado,
nombres auxiliares o declaraciones anteriores con el mismo nombre Python.
En esos casos se asigna un nombre único como `_c_var_2`.

Al entrar y salir de cada bloque se activa o cierra su ámbito. Una variable
interior puede ocultar otra exterior sin cambiar sus referencias. La
declaración se registra antes de analizar el inicializador; por eso
`int x = x;` no toma accidentalmente el valor de una variable exterior.

Una declaración sin inicializador no produce una asignación Python. No se
inventa un cero. Una asignación válida marca la variable como inicializada.

Cada `if` guarda los estados de entrada y de la primera rama. La alternativa
se analiza desde el estado de entrada. Al reunir las ramas se conserva solo
la inicialización garantizada en las rutas que continúan. Un `return` cierra
su ruta. No se evalúan condiciones constantes ni se intenta demostrar
relaciones entre condiciones de distintos `if`.

## Acciones de Bison y generación

| Producción | Acción |
|---|---|
| `directiva_inclusion` | Reconoce la directiva existente; no emite un include Python. |
| `cabecera_declaracion` | Registra nombre y tipo antes del inicializador. |
| `declaracion` | Registra sin asignar, o convierte y genera la inicialización. |
| `asignacion` | Busca el símbolo, convierte al tipo de destino y genera una línea. |
| `factor` | Resuelve variables, valida literales y conserva agrupación y signo. |
| `termino`, `expresion` | Construyen expresiones tipadas sin calcular resultados. |
| `condicion` | Aplica las conversiones numéricas y construye la comparación. |
| `inicio_if`, `sentencia_if`, `parte_else` | Controlan estados y construyen `if`, `elif` y `else`. |
| `lista_argumentos`, `llamada_printf` | Conservan argumentos, validan formatos y construyen la impresión. |
| `sentencia_return` | Convierte el resultado de `main` a entero y cierra la ruta. |
| `bloque`, `lista_sentencias` | Controlan ámbitos y unen las instrucciones en orden. |
| `definicion_funcion`, `programa` | Entregan el cuerpo completo para su publicación. |

Se conservaron las alternativas de `expresion`, `termino` y `factor`.
Las pequeñas producciones auxiliares no agregan construcciones al lenguaje.
Los destructores también liberan listas de argumentos y contextos de `if`
si el análisis se interrumpe.

El generador trabaja con fragmentos de texto, sin construir un árbol
sintáctico completo. Al envolver un bloque se añaden cuatro espacios por
nivel. Si no hay instrucciones emitidas, se genera `pass`.

## Equivalencia numérica y salida

- La división entre enteros usa `_c_idiv`, que trunca hacia cero también
  con operandos negativos y no convierte primero a real.
- La conversión real a entero usa `int(...)`.
- La conversión a C `float` y los resultados de operaciones de ese tipo
  usan `_c_f32`, basado en `struct` de la biblioteca estándar de Python.
- Los literales reales sin sufijo se consideran internamente `double`.
  Se convierten los operandos al tipo común antes de operar o comparar.
- Los enteros octales que reconoce el texto se validan y representan en
  decimal. Esto normaliza el literal; no evalúa expresiones.
- `printf` admite `%d`, `%f`, `%.Nf` y `%%`, con cantidad y tipos verificados.
  Se admiten los escapes `\n`, `\t`, `\r`, `\\` y `\"`.
- Se utiliza `print(..., end="")` para conservar los saltos originales.
- `return` conserva la terminación anticipada y el resultado se entrega a
  `SystemExit`. Llegar al final de `main` también termina con éxito.
- Los nombres y los paréntesis se conservan o simplifican según sea seguro.
  Solo se incluyen los auxiliares Python utilizados.

No se ejecutan las expresiones durante la traducción. Se probó expresamente
una división en una rama que no se ejecuta y se verificó que permanece en
el archivo Python.

## Publicación segura

El archivo definitivo no se abre para escribir mientras se analiza C.
Primero deben terminar correctamente `yyparse()` y las comprobaciones
léxicas, sintácticas y semánticas.

Después se crea exclusivamente un temporal junto al destino. El código se
escribe mediante `FILE *archivo_salida` y `fprintf()`. Se comprueban escritura,
vaciado y cierre antes de reemplazar el destino mediante `MoveFileExA` de
Windows. Si falla, se elimina únicamente el temporal creado por esta operación.

Una salida anterior permanece intacta cuando falla el análisis. Se rechaza
usar el archivo de entrada como salida, incluso mediante un enlace físico.
No se crean directorios automáticamente. Esta publicación está implementada
para Windows y MSYS2 UCRT64, usando las rutas nativas del ejecutable C.

## Validación realmente ejecutada

Se compiló mediante `bash compilar.sh`, con estas herramientas resueltas
explícitamente desde MSYS2:

```text
/usr/bin/bash
/usr/bin/bison
/usr/bin/flex
/ucrt64/bin/gcc
```

La compilación devolvió código 0, sin advertencias de Flex, Bison o GCC.
Bison mantuvo `-Wall -Werror=conflicts-sr -Werror=conflicts-rr`: no hubo
conflictos shift/reduce ni reduce/reduce. GCC mantuvo `-Wall -Wextra`.

| Grupo | Esperado | Obtenido | Estado |
|---|---|---|---|
| Regresiones de la etapa 3 | 33 aprobadas | 33/33 | OK |
| Tres originales, copiados con otros nombres | Sintaxis Python válida y C=Python | 3/3 | OK |
| Otros programas válidos de la batería | Sintaxis Python válida y C=Python | 34/34 | OK |
| Entradas inválidas, con destino nuevo y anterior | Rechazo sin publicar ni sobrescribir | 24/24 | OK |
| Reemplazo de una salida válida | Contenido nuevo completo | 1/1 | OK |
| Misma ruta y enlace físico a la entrada | Rechazo y original intacto | 2/2 | OK |
| Destino directorio, ruta inexistente y archivo de solo lectura | Fallo sin temporales incompletos | 3/3 | OK |
| Integridad durante la batería nueva | SHA-256 sin cambios | 29 archivos intactos | OK |
| Integridad contra el respaldo inicial | SHA-256 sin cambios | 28 archivos intactos | OK |

La etapa 4 tiene **68/68 comprobaciones aprobadas**. Las entradas inválidas
se probaron dos veces cada una: con destino nuevo y con salida anterior.
Los 37 programas válidos se compilaron con GCC, se ejecutaron junto con sus
traducciones y coincidieron en salida y código de terminación. Se normalizó
CRLF/LF al comparar la salida de consola. Los retornos anticipados con códigos
7 y 3 también coincidieron.

Las fuentes adicionales y todos los productos de la batería se crearon en
una carpeta temporal fuera del proyecto. No se conservaron traducciones
definitivas de los tres originales. El ejecutor imprime la tabla individual
completa en cada ejecución.

Los programas válidos verificaron división negativa y grande, asociación,
precedencia, conversiones, redondeo float, promoción de operandos, sombreado,
palabras Python, ámbitos hermanos, inicialización entre ramas, `elif`, `pass`,
retornos, porcentajes, escapes, octales y cinco programas con nombres y
valores distintos. Las pruebas negativas cubrieron declaraciones, ámbitos,
inicialización, formatos, escapes, límites y errores léxicos/sintácticos.

## Ejemplo generado automáticamente

Se creó un programa adicional con `puntos = 8`, `extra = 5`, una variable
`total` sin inicialización y un `if` que imprime el total cuando supera 10.
El traductor produjo el siguiente archivo, sin editarlo manualmente:

```python
def main():
    puntos = 8
    extra = 5
    total = puntos + extra
    if total > 10:
        print("Total: %d\n" % (total,), end="")
    else:
        print("Sin resultado\n", end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

La generación devolvió código 0. La ejecución Python devolvió código 0 e
imprimió `Total: 13` seguido de un salto de línea. Los archivos de esta
demostración están temporalmente en:

```text
%TEMP%\ps-demo-etapa4-bcyv4ux5
```

## Comandos para comprobarlo

Desde la raíz del proyecto en MSYS2 UCRT64:

```bash
bash compilar.sh
python -B pruebas/probar_etapa3.py
python -B pruebas/probar_etapa4.py
```

La invocación local de Python se representa aquí con el nombre disponible en PATH:

```bash
python -B pruebas/probar_etapa4.py
```

Una traducción individual puede hacerse en un temporal, sin adelantar los
tres archivos definitivos de la etapa 5:

```bash
carpeta_prueba=$(mktemp -d)
./compilador.exe ejemplos/ejemplo1.c "$carpeta_prueba/prueba.py"
python -B "$carpeta_prueba/prueba.py"
```

## Limitaciones de la primera versión

- Se conserva el subconjunto existente: una inclusión, `int main()`,
  declaraciones individuales `int`/`float`, aritmética, comparaciones,
  bloques de condicionales, `printf` y `return`. No hay funciones adicionales,
  ciclos, operadores lógicos, arreglos ni conversiones explícitas de C.
- Hay hasta 100 declaraciones por programa, incluyendo las de ámbitos cerrados.
- El análisis de inicialización es conservador: puede rechazar casos válidos
  que dependan de demostrar condiciones constantes o relaciones entre ellas.
- Los literales enteros deben caber en `int`; no se agregan los tipos enteros
  más amplios que podría seleccionar un compilador de C completo.
- La equivalencia numérica se limita a cálculos definidos dentro de los rangos
  de los tipos, con números finitos y el redondeo ordinario del entorno UCRT64.
  No se emulan desbordamientos, modos especiales ni todos los casos extremos
  de la aritmética flotante. No se demuestra ausencia de división por cero
  durante la ejecución.
- Los formatos y escapes son los indicados arriba. Se exige coincidencia
  exacta de argumentos; no se implementa toda la biblioteca `printf`.
- La comparación de códigos de terminación se centra en el rango portable
  0–127. La publicación utiliza las interfaces de archivos de Windows.

No se inició la etapa 5. La documentación general del proyecto podrá
actualizarse integralmente en la etapa de documentación final.
