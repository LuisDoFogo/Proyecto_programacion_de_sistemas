# Informe de la etapa 3

> Copia pública anonimizada: las rutas locales se sustituyeron por
> marcadores y las invocaciones personales de Python por `python`.
> Los resultados de las pruebas no se modificaron. Se conserva una
> copia íntegra del informe original fuera del repositorio.

Se adaptaron los valores semánticos de Flex y Bison y se corrigieron los
defectos léxicos. Esta etapa conserva el modo de análisis: todavía no genera
archivos Python ni comprueba declaraciones, ámbitos o inicialización.

## Protección del proyecto

Antes de editar se ejecutaron `git status --short` y `git diff`. Los tres
archivos existentes que se modificaron no tenían cambios locales. El PDF y
los ejemplos del profesor ya aparecían sin seguimiento; se conservaron.

El respaldo de las fuentes originales y de los artefactos que la compilación
sobrescribe está fuera del proyecto, en:

```text
%TEMP%\ProyectoProgramacionDeSistemas-etapa3-20261113-220118
```

También se guardaron el estado inicial de Git y las sumas SHA-256 de los 19
archivos originales de `ejemplos/` y `pruebas/`. Después de las pruebas se
comprobó que esos 19 archivos permanecen idénticos. No hubo commits ni push.

## Archivos

Modificados:

- `src/analizador_c.l`: lexemas, ubicaciones, cadenas, comentarios y errores.
- `src/parser.y`: unión semántica, tokens tipados, acciones y destructores.
- `compilar.sh`: enlace de los auxiliares y rechazo de conflictos de Bison.

Creados:

- `src/traduccion.h` y `src/traduccion.c`: tipos y auxiliares de expresiones.
- `pruebas/valido_cadenas_escapadas.c`.
- `pruebas/invalido_cadena_sin_cerrar.c`.
- `pruebas/invalido_comentario_sin_cerrar.c`.
- `pruebas/invalido_caracter_entre_printf.c`.
- `pruebas/invalido_texto_entre_cadenas.c`.
- `pruebas/valores_semanticos.c`: comprobaciones de los auxiliares en C.
- `pruebas/probar_etapa3.py`: ejecutor de las regresiones, sin dependencias externas.
- `pruebas/ETAPA3.md`: este informe.

`bash compilar.sh` regenera `parser.tab.c`, `parser.tab.h`, `lex.yy.c` y
`compilador.exe`, que ya estaban excluidos de Git.

## Valores semánticos

Flex copia `yytext` en `yylval.texto` para `ID`, `INT_LITERAL`,
`FLOAT_LITERAL` y `STRING`. Cada copia pertenece al parser hasta que se
transfiere a una expresión o se libera.

La directiva `%union` contiene texto, operadores, un tipo y una `Expresion`.
Las directivas `%token` y `%type` indican qué miembro corresponde a cada
token o producción. Las acciones de `expresion`, `termino` y `factor` usan
`$1`, `$2`, `$3` y `$$` para propagar el texto y sus atributos. Sus
alternativas gramaticales se conservan.

Una expresión contiene:

| Campo | Significado |
|---|---|
| `texto` | Expresión aritmética conservada, sin evaluar. |
| `tipo` | `T_INT`, `T_FLOAT`, `T_DOUBLE` o `T_DESCONOCIDO`. |
| `precedencia` | Nivel aditivo, multiplicativo, unario o atómico. |

Los identificadores conservan su nombre original y tienen tipo desconocido
hasta que se incorpore la tabla de símbolos. Los literales reales sin sufijo
usan `T_DOUBLE`; esto no agrega declaraciones `double` al lenguaje.

Los constructores conservan la agrupación necesaria y evitan paréntesis
redundantes. Por ejemplo, distinguen `10 - (4 - 2)` de `10 - 4 - 2` y
mantienen `(x + 1) * 2`. No convierten divisiones a Python ni insertan
conversiones numéricas todavía. Tampoco evalúan una división como `1 / 0`.

Las acciones liberan los valores al terminar de utilizarlos. Los
`%destructor` liberan lexemas y expresiones descartados por un error.
`yylex_destroy()` libera los recursos del scanner después del cierre del
archivo de entrada. Si falta memoria, se informa un error y no se anuncia éxito.

## Correcciones léxicas

- Una cadena no puede consumir comillas sin escapar ni otras instrucciones.
- Se reconocen comillas y barras escapadas sin perder el lexema original.
- Una comilla que no inicia una cadena completa produce un error localizado.
- El estado exclusivo `COMENTARIO` detecta comentarios de bloque sin cierre.
- Los bytes NUL y los caracteres inválidos producen `ERROR_LEXICO`.
- Un contador independiente impide anunciar éxito si ocurrió un error léxico.
- `%locations` y el seguimiento del lexer proporcionan línea y columna,
  también después de comentarios y al alcanzar el fin de archivo.

El reconocimiento de escapes no constituye todavía su validación semántica:
la conversión y el perfil de escapes/formato de `printf` corresponden a la
generación de Python.

## Compilación ejecutada

Se ejecutó explícitamente el Bash de MSYS2 con `MSYSTEM=UCRT64`.
Las herramientas utilizadas fueron:

```text
/usr/bin/bash
/usr/bin/bison
/usr/bin/flex
/ucrt64/bin/gcc
```

El comando fue `bash compilar.sh`. La compilación final devolvió código 0:

```text
[1/3] Generando el parser con Bison...
[2/3] Generando el lexer con Flex...
[3/3] Compilando...
Listo: compilador.exe
```

No se emitieron advertencias de Bison, Flex o GCC. Bison se ejecutó con
`-Wall -Werror=conflicts-sr -Werror=conflicts-rr`: la compilación confirma
que no hubo conflictos shift/reduce ni reduce/reduce. GCC utilizó
`-Wall -Wextra`; la prueba auxiliar en C añadió `-Werror`.

El primer intento de compilación dentro del aislamiento no pudo escribir
los temporales de GCC en `<MSYS2>\tmp`. Se repitió con permiso fuera del
aislamiento y terminó correctamente; el intento bloqueado no se contabiliza
como una compilación aprobada.

## Resultados reales

Se ejecutó el programa de pruebas después de la recompilación final.
Los valores de las dos columnas centrales son códigos de terminación.
También se comprueban los diagnósticos y la ausencia de mensajes de éxito
en todos los casos inválidos.

| Prueba | Esperado | Obtenido | Estado |
|---|---:|---:|---|
| ejemplos/ejemplo1.c | 0 | 0 | OK |
| ejemplos/ejemplo2.c | 0 | 0 | OK |
| ejemplos/ejemplo3.c | 0 | 0 | OK |
| pruebas/valido_comentarios.c | 0 | 0 | OK |
| pruebas/invalido_sin_punto_y_coma.c | 1 | 1 | OK |
| pruebas/invalido_parentesis.c | 1 | 1 | OK |
| pruebas/valido_cadenas_escapadas.c | 0 | 0 | OK |
| pruebas/invalido_cadena_sin_cerrar.c | 1 | 1 | OK |
| pruebas/invalido_comentario_sin_cerrar.c | 1 | 1 | OK |
| pruebas/invalido_caracter_entre_printf.c | 1 | 1 | OK |
| pruebas/invalido_texto_entre_cadenas.c | 1 | 1 | OK |
| stdin sin argumentos | 0 | 0 | OK |
| precedencia y menos unario | 0 | 0 | OK |
| expresion sin constant folding | 0 | 0 | OK |
| comentario al final sin cerrar | 1 | 1 | OK |
| cadena al final sin cerrar | 1 | 1 | OK |
| cadena con escape al final del archivo | 1 | 1 | OK |
| comillas escapadas sin cierre real | 1 | 1 | OK |
| byte NUL al final | 1 | 1 | OK |
| byte NUL inicial | 1 | 1 | OK |
| byte NUL en comentario de linea | 1 | 1 | OK |
| byte NUL en comentario de bloque | 1 | 1 | OK |
| byte NUL en cadena | 1 | 1 | OK |
| caracter invalido al final | 1 | 1 | OK |
| comentarios con asteriscos | 0 | 0 | OK |
| lineas CRLF | 0 | 0 | OK |
| comentario multilinea y ubicacion | 1 | 1 | OK |
| EOF y ubicacion | 1 | 1 | OK |
| identificador pendiente tras error | 1 | 1 | OK |
| expresion pendiente tras error | 1 | 1 | OK |
| STRING sigue sin ser argumento aritmetico | 1 | 1 | OK |
| alcance original: analisis sin tabla de simbolos | 0 | 0 | OK |
| valores semanticos (C) | 0 | 0 | OK |

Resultado: **33/33 pruebas aprobadas**. Las entradas binarias y los casos
adicionales se construyen en memoria, sin añadir bytes NUL a las fuentes.
La prueba auxiliar se compila y ejecuta en una carpeta temporal fuera del
proyecto. No se ejecutaron traducciones Python ni comparaciones C/Python.

## Cómo repetir las comprobaciones

Desde la raíz del proyecto, en la terminal MSYS2 UCRT64:

```bash
bash compilar.sh
python -B pruebas/probar_etapa3.py
```

Para ejecutar casos individuales y consultar el código de terminación:

```bash
./compilador.exe ejemplos/ejemplo1.c
echo $?
./compilador.exe pruebas/invalido_cadena_sin_cerrar.c
echo $?
./compilador.exe < pruebas/valido_comentarios.c
echo $?
```

Si Python de Windows no está en el PATH de esa terminal, en este equipo puede
invocarse el ejecutor así:

```bash
python -B pruebas/probar_etapa3.py
```

## Pendientes para la siguiente etapa

No se generan archivos Python. Todavía no hay tabla de símbolos, control de
ámbitos, inicialización, comprobación de formatos ni conversiones numéricas.
Por eso una variable no declarada sigue siendo aceptada en el modo actual
de análisis, como antes; esto se verifica expresamente en las pruebas.

La conservación de nombres y la precedencia ya tienen soporte en las
expresiones. La resolución de conflictos de identificadores, los auxiliares
Python y la publicación segura del archivo se implementarán en la etapa 4,
únicamente después de la autorización del usuario.
