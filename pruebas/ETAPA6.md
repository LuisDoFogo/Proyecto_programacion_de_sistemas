# Etapa 6: pruebas y validación integral

> Copia pública anonimizada: las rutas locales se sustituyeron por
> marcadores y las invocaciones personales de Python por `python`.
> Los resultados de las pruebas no se modificaron. Se conserva una
> copia íntegra del informe original fuera del repositorio.

## Objetivo y alcance

Validar el transpilador de C a Python dentro del subconjunto aprobado,
comprobar su compilación y ejecución, repetir las regresiones existentes y
verificar su traslado a otra ruta de Windows con espacios. No se añadieron
funcionalidades, producciones gramaticales ni cambios al código fuente.

Se creó únicamente este informe dentro del proyecto. La compilación
regeneró los artefactos habituales, excluidos de Git. Las demás evidencias,
copias y traducciones de validación se guardaron fuera del proyecto.

## Protección del proyecto

Se ejecutó `git status --short` antes de comenzar. Se conservaron las
modificaciones aprobadas de las etapas anteriores y todos los archivos sin
seguimiento. No se realizaron `git add`, commits ni push.

Se registraron las huellas SHA-256 de 43 archivos antes de validar:

- Los tres programas originales y todos los ejemplos del profesor.
- El PDF de referencia.
- Las fuentes del transpilador y `compilar.sh`.
- Las pruebas y los informes anteriores.
- Los tres Python definitivos y la documentación existente.

Al finalizar, los **43/43 archivos permanecieron idénticos**. No se
eliminaron pruebas ni se modificaron manualmente los Python generados.
El manifiesto excluye `.git/` y los artefactos que el proceso de compilación
debe regenerar: `compilador.exe`, `parser.tab.c`, `parser.tab.h` y `lex.yy.c`.
Este informe es nuevo y no formaba parte del manifiesto inicial.

## Herramientas y versiones realmente utilizadas

| Herramienta | Versión observada | Procedencia de esta ejecución |
|---|---|---|
| Bash | GNU bash 5.3.15(1)-release, x86_64-pc-cygwin | `<MSYS2>\usr\bin\bash.exe` |
| GNU Bison | 3.8.2 | `<MSYS2>\usr\bin\bison.exe` |
| Flex | 2.6.4 | `<MSYS2>\usr\bin\flex.exe` |
| GCC | 16.2.0, Rev3 de MSYS2 | `<MSYS2>\ucrt64\bin\gcc.exe` |
| Destino de GCC | `x86_64-w64-mingw32` | Resultado de `gcc -dumpmachine` |
| Python | 3.14.7 | Python resuelto con el entorno UCRT64 de esta validación |

Se establecieron `MSYSTEM=UCRT64` y el orden de herramientas de MSYS2.
Dentro del Bash correcto, `command -v` devolvió:

```text
/usr/bin/bash
/usr/bin/bison
/usr/bin/flex
/ucrt64/bin/gcc
```

Las rutas de esta tabla identifican las herramientas utilizadas en este
equipo; no son rutas incorporadas al transpilador. Los compañeros necesitan
su propia instalación de MSYS2 UCRT64 y Python 3 disponibles en el entorno.
No se realizó una matriz de versiones ni una instalación nueva en otro equipo.

## Compilación del proyecto original

Se ejecutó el comando solicitado desde la raíz:

```bash
bash compilar.sh
```

Resultado real:

```text
[1/3] Generando el parser con Bison...
[2/3] Generando el lexer con Flex...
[3/3] Compilando...
Listo: compilador.exe
```

| Comprobación | Resultado | Estado |
|---|---|---|
| Bison genera `parser.tab.c` y `parser.tab.h` | Código 0, sin diagnósticos | Cumple |
| Flex genera `lex.yy.c` | Código 0, sin diagnósticos | Cumple |
| GCC compila y enlaza | Código 0, sin diagnósticos | Cumple |
| Ejecutable resultante | `compilador.exe` presente | Cumple |
| Advertencias de compilación | Ninguna en stdout ni stderr | Cumple |
| Conflictos shift/reduce y reduce/reduce | Ninguno | Cumple |

Se conservaron `-Wall` de Bison, `-Wall -Wextra` de GCC y los indicadores
`-Werror=conflicts-sr -Werror=conflicts-rr`. Estos últimos hacen fallar la
compilación ante cualquiera de esas clases de conflicto.

La compilación desde cero se comprobó por separado en la copia limpia
descrita en la sección de portabilidad, sin borrar artefactos del original.

## Regresiones existentes

Se ejecutó una vez cada batería, después de compilar el proyecto original:

```bash
python -B pruebas/probar_etapa3.py
python -B pruebas/probar_etapa4.py
```

| Batería | Esperado | Resultado real | Código del ejecutor |
|---|---:|---:|---:|
| Etapa 3 | 33 aprobadas | 33/33 | 0 |
| Etapa 4 | 68 aprobadas | 68/68 | 0 |
| Total de regresiones | 101 aprobadas | 101/101 | 0 |

Ambos ejecutores terminaron sin salida de error. No se cambiaron los casos
existentes ni se añadió una batería que repitiera las mismas entradas.

Las regresiones comprueban el modo original de análisis, lexemas y
ubicaciones, precedencia, errores léxicos y sintácticos, símbolos, tipos,
ámbitos, inicialización, conversiones, formatos, retornos y publicación de
salidas. La batería de etapa 4 volvió a ejecutar sus **37 comparaciones
C/Python**, además de sus rechazos y controles de archivos.

## Validación del proceso completo y reproducibilidad

Se utilizó el ejecutable recién compilado para traducir nuevamente los tres
originales hacia archivos de una carpeta temporal externa. Los productos
resultaron idénticos, byte por byte, a los Python definitivos. No se
sobrescribieron los archivos de `salida_python/`.

Se comprobó la sintaxis con `ast.parse` sobre los archivos definitivos,
los regenerados y los generados por la copia portátil. Después se
compilaron los tres originales con:

```bash
gcc -std=c11 -Wall -Wextra archivo_original.c -o ejecutable_temporal.exe
```

Las tres compilaciones C terminaron con código 0 y sin advertencias. Sus
ejecutables estuvieron en un directorio temporal externo, eliminado al
terminar. Se ejecutaron los C, los Python definitivos y los Python generados
desde la copia trasladada, estos últimos desde otra carpeta de trabajo.

| Programa | Salida observada en C y en ambos Python | Código C | Código Python definitivo | Código Python trasladado | Resultado |
|---|---|---:|---:|---:|---|
| ejemplo1 | Suma 13, resta 3 y multiplicación 40 | 0 | 0 | 0 | Bytes idénticos |
| ejemplo2 | Promedio 8.33 y mensaje de aprobación | 0 | 0 | 0 | Bytes idénticos |
| ejemplo3 | Mensaje de número negativo | 0 | 0 | 0 | Bytes idénticos |

Salidas exactas:

```text
La suma es: 13
La resta es: 3
La multiplicacion es: 40
```

```text
El promedio es: 8.33
El alumno aprobo
```

```text
El numero es negativo
```

Cada línea termina en CRLF en este entorno, incluida la última. La igualdad
se comprobó directamente sobre los bytes capturados, sin normalizar los
finales de línea. No hubo salida de error. Coincidieron los formatos, el
redondeo mostrado, las ramas ejecutadas y los códigos de terminación.

| Archivo definitivo | SHA-256 idéntico en las regeneraciones |
|---|---|
| `salida_python/ejemplo1.py` | `96b7a60296781ddcd8159cd7698e7432b1babe8a35512478bf72dfd5f0bbd2c5` |
| `salida_python/ejemplo2.py` | `1175c3e6997bb4eb2c34d10328aea2b42b0d8217d2968bbbf5de7ace51d9cbd0` |
| `salida_python/ejemplo3.py` | `48f224689158f17025b7875e33a7f771ae4a5c97cd824bcdcef358ba522d5454` |

La ejecución de cada original recorre las ramas determinadas por sus valores
actuales; las demás rutas se cubren con los programas adicionales de la
batería existente. No se cambiaron los originales para forzar nuevas ramas.

## Portabilidad en otra ruta de Windows

Se copió la carpeta de trabajo, incluyendo fuentes y materiales sin
seguimiento, a:

```text
%TEMP%\ProyectoProgramacionDeSistemas-etapa6-3po3suna\entrega con espacios\Proyecto de Sistemas
```

Antes de compilar, la copia **no tenía** `compilador.exe`, `parser.tab.c`,
`parser.tab.h` ni `lex.yy.c`. Se excluyeron también `.git/` y las cachés de
Python. No se borró ningún archivo del proyecto original.

El Bash de MSYS2 ejecutó `bash` con la ruta completa de `compilar.sh`,
entre comillas, desde una carpeta de trabajo diferente a la del proyecto.
Esto comprobó tanto los espacios de la ruta como el cambio de directorio
que realiza el script.

| Comprobación en la copia | Resultado real | Estado |
|---|---|---|
| Generación sin artefactos previos | Se crearon los cuatro artefactos esperados | Cumple |
| Compilación desde otra carpeta | Código 0, sin advertencias ni errores | Cumple |
| Entrada C con espacios en su ruta completa | Los tres originales se leyeron correctamente | Cumple |
| Salida con espacios en carpeta y nombre | Se generaron `generado 1.py`, `generado 2.py` y `generado 3.py` | Cumple |
| Reproducibilidad de las tres traducciones | Mismos bytes que los definitivos | Cumple |
| Ejecución fuera de la carpeta copiada | Las tres ejecuciones coincidieron con C | Cumple |

No se repitieron las 101 regresiones en la copia: ya se ejecutaron sobre las
mismas fuentes en el original. La copia se utilizó para las comprobaciones
específicas de compilación limpia, rutas, traslado y reproducción.

### Dependencias y límites de esta comprobación

Se revisaron diez archivos operativos: fuentes, `compilar.sh`, los dos
ejecutores de pruebas y los tres Python definitivos. No contienen rutas
absolutas personales, nombres de usuario ni una ubicación fija de MSYS2 o
Python. Los ejecutores obtienen el proyecto desde `__file__`, usan el
intérprete actual y admiten `CC` o `gcc`.

La publicación utiliza interfaces de Windows, incluyendo `MoveFileExA`,
`CreateFileA` y funciones del CRT. La inspección del ejecutable mostró
importaciones de `KERNEL32.dll` y de la familia `api-ms-win-crt-*` de UCRT.
No mostró una DLL particular del usuario. Los Python generados usan solo
Python y, cuando corresponde, `struct` de su biblioteca estándar.

La prueba demuestra traslado dentro de Windows con esta instalación de
herramientas. No demuestra funcionamiento en Linux/macOS, en todas las
versiones de Windows, con todas las codificaciones de rutas ni con cualquier
versión de las herramientas. Tampoco sustituye una instalación en el equipo
de otro compañero.

Los informes anteriores incluyen rutas históricas de evidencias y ejemplos
de comandos con `<PERFIL_WINDOWS>` o `Python313`. No son dependencias del
transpilador y no se consultan durante su ejecución. Esos ejemplos deberán
presentarse de forma general en la etapa de documentación definitiva.

## Matriz de cumplimiento académico

La referencia principal es el documento
[04 05 PS Construcción de un compilador](<../04 05 PS Construcción de un compilador (1).pdf>)
y los cinco ejemplos docentes de `ejemplos/`. La numeración de páginas
siguiente corresponde a las 38 páginas del PDF.

| Elemento | Implementación | Archivos relacionados | Referencia docente | Evidencia de esta validación | Estado |
|---|---|---|---|---|---|
| Analizador léxico | Reglas de tokens, cadenas, comentarios, ubicación y errores | `src/analizador_c.l` | PDF pp. 6 y 9–13; lexers de ejemplos 01–05 | Flex genera el scanner; pasan las regresiones léxicas y las lecturas de los tres originales | Cumple |
| Analizador sintáctico | `yyparse()` verifica el orden de tokens y llama a las acciones | `src/parser.y` | PDF pp. 3–4, 10 y 12–14; parsers 01–05 | Bison compila sin conflictos; se aceptan y rechazan las entradas previstas | Cumple |
| Gramática | Subconjunto de C, listas y niveles `expresion`, `termino`, `factor` | `src/parser.y` | PDF pp. 3–4 y 28; precedencia en ejemplos 01, 02 y 05 | Compilación con conflictos tratados como errores; regresiones de precedencia y asociación aprobadas | Cumple en el subconjunto |
| Valores semánticos | `yytext`, `yylval`, `%union`, tokens/tipos, `$1`, `$2`, `$3`, `$$` y destructores | `src/analizador_c.l`, `src/parser.y`, `src/traduccion.h` | PDF pp. 11 y 16–18, 22 y 26–27; ejemplos 01–02 y 04–05 | Prueba auxiliar C y casos de lexemas, tipos, agrupación y errores aprobados | Cumple |
| Acciones semánticas | Construcción de expresiones y sentencias al reducir producciones | `src/parser.y`, `src/traduccion.c` | PDF pp. 4, 12 y 17–19; ejemplos 03–05 | Las tres traducciones se regeneran de forma idéntica y se ejecutan correctamente | Cumple |
| Tabla de símbolos | Arreglo con nombre, tipo, ámbito, actividad e inicialización | `src/traduccion.c`, `src/traduccion.h` | PDF pp. 30 y 36–37; ejemplos 02 y 05 | Se comprueban duplicados, referencias, sombreado, ámbitos e inicialización entre ramas | Cumple en el alcance aprobado |
| Generación automática de código | Fragmentos construidos por Bison y escritura con `FILE *archivo_salida` y `fprintf()` | `src/parser.y`, `src/traduccion.c` | PDF pp. 19 y 32–36; ejemplos 03–05 | Compilación limpia y generación real desde dos ubicaciones, sin editar los productos | Cumple |
| Traducción de C a Python | Equivalencias de tipos, operaciones, impresión, condicionales y retorno | `src/parser.y`, `src/traduccion.c`, `salida_python/` | Adaptación de la técnica de los ejemplos 03–05; Python como destino fue solicitado por el usuario | Tres archivos definitivos válidos, reproducibles y ejecutables | Cumple en el subconjunto |
| Ejecución equivalente de los tres originales | Conversión numérica, formatos, selección de ramas y terminación | `ejemplos/ejemplo1.c`, `ejemplo2.c`, `ejemplo3.c` y sus `.py` | Objetivo del usuario; el material docente aporta los mecanismos de traducción | Las tres comparaciones de bytes y códigos coinciden; además pasan las 37 comparaciones de la batería | Cumple para las entradas validadas |
| Detección y manejo de errores | Diagnósticos léxicos, sintácticos, semánticos y de archivos; no se publica salida incorrecta ante esos fallos | `src/analizador_c.l`, `src/parser.y`, `src/traduccion.c` | PDF p. 14 y ejemplos 01, 02 y 05 para diagnósticos; publicación segura requerida por el usuario | Rechazos de las baterías y fallos de lectura/argumentos; códigos no cero y archivos protegidos | Cumple en los casos probados |

El profesor muestra generación de C desde pseudocódigo; el destino Python
y la conservación del comportamiento de los tres programas son decisiones
del proyecto solicitadas por el usuario. Los ejemplos docentes no establecen
por sí mismos el uso de `_c_f32`, el análisis de inicialización entre ramas
ni la publicación mediante temporal. Estas son adaptaciones para satisfacer
los requisitos aprobados, no requisitos adicionales atribuidos al profesor.
El ejemplo 05 muestra optimización de valores; aquí se omitió por petición
expresa del usuario, conservando las expresiones para su ejecución.

## Controles adicionales sin duplicar las baterías

Se añadieron únicamente comprobaciones puntuales de la interfaz que no
repetían los casos de la batería semántica:

| Control | Esperado | Resultado observado |
|---|---|---|
| Archivo de entrada inexistente | Error y ausencia de salida Python | Código 1; no se creó salida |
| Exceso de argumentos | Mensaje de uso y terminación fallida | Código 1; no se publicó salida |

La revisión estática también confirmó el recorrido del proceso:

| Paso | Implementación comprobada | Evidencia |
|---|---|---|
| Lectura de C | `yyin = fopen(...)` | Tres lecturas correctas y rechazo de archivo inexistente |
| Léxico y sintaxis | `yylex()` y `yyparse()` | Compilación limpia y regresiones |
| Transferencia semántica | Unión, atributos y estructuras compartidas | Unidad C y resultados calculados en la ejecución Python |
| Símbolos y comprobaciones | Declaración, búsqueda y estados por rama | Batería semántica aprobada |
| Construcción del programa | Acciones de Bison y auxiliares C | Regeneración idéntica de los tres productos |
| Escritura | `publicar_programa()`, `FILE *archivo_salida`, `fprintf()` | Archivos creados en temporales y rutas con espacios |
| Ejecución | Punto de entrada Python y `SystemExit(main())` | Salidas y códigos idénticos en original y copia |

## Problemas e incidencias encontradas

1. **Selección de Bash al automatizar desde Windows.** En el primer intento,
   invocar `bash` por nombre desde Python seleccionó el ejecutable de WSL y
   devolvió `Bash/E_ACCESSDENIED` antes de compilar. Se corrigió la llamada de
   validación para utilizar la ruta absoluta de Bash de MSYS2. No se cambió
   el proyecto. Ese intento no cuenta como compilación aprobada.
2. **Documentación general anterior a la traducción.** `README.md` todavía
   describe un analizador sin semántica; `GRAMATICA.md` menciona
   `IF_WITHOUT_ELSE` y producciones que no coinciden con el parser actual;
   `PRESENTACION.md` utiliza nombres de pruebas inexistentes. No se editaron:
   su actualización corresponde a la etapa 7.
3. **Archivos nuevos sin seguimiento en Git.** La copia portátil incluyó toda
   la carpeta de trabajo, también `src/traduccion.c`, `src/traduccion.h`,
   pruebas, materiales y Python que aparecen con `??`. La prueba no fue un
   clon del repositorio. Una entrega deberá incluir esos archivos; el estado
   del repositorio versionado por sí solo no representa todo el proyecto
   validado. No se añadieron ni publicaron archivos en Git.

No se encontró un defecto funcional dentro de las pruebas y el alcance
validados que requiriera modificar las fuentes. Por tanto, no se solicitó
autorización para cambios de código ni se realizó ninguno.

## Limitaciones conocidas

- Se reconoce el subconjunto acordado, con una inclusión y `int main()`;
  no se amplía a ciclos, arreglos, funciones adicionales u operadores lógicos.
- Hay hasta 100 declaraciones por programa, incluyendo ámbitos cerrados.
- La comprobación de inicialización es conservadora y no demuestra relaciones
  entre condiciones de diferentes ramas ni evalúa condiciones constantes.
- Los formatos admitidos son `%d`, `%f`, `%.Nf` y `%%`, con correspondencia
  exacta de argumentos. Los escapes admitidos son `\n`, `\t`, `\r`, `\\` y `\"`.
- La equivalencia numérica se limita al perfil finito y los rangos acordados
  para UCRT64. No se garantiza para desbordamientos, casos extremos de
  flotantes, modos especiales de redondeo o comportamiento indefinido de C.
- No se demuestra ausencia de división por cero durante la ejecución; tampoco
  se evalúan expresiones anticipadamente para intentar detectarla.
- El modo de un archivo conserva el análisis sintáctico original; el modo
  de entrada y salida activa la semántica necesaria para traducir.
- Se validó Windows con las versiones indicadas y códigos de terminación
  ordinarios. No se certificaron todos los sistemas, versiones o rutas Unicode.

## Evidencias y comandos

Los manifiestos SHA-256, versiones, logs completos y resultados JSON están en:

```text
%TEMP%\ProyectoProgramacionDeSistemas-etapa6-3po3suna
```

| Evidencia | Contenido |
|---|---|
| `sha256-inicial.json`, `sha256-final.json` | 43 archivos originales idénticos |
| `git-status-inicial.txt` | Cambios preexistentes y archivos sin seguimiento |
| `versiones.json` y logs de versiones | Herramientas efectivamente ejecutadas |
| `compilacion-original.log` | Compilación normal sin diagnósticos |
| `compilacion-limpia-copia.log`, `portabilidad.json` | Compilación desde cero y tres traducciones en otra ruta |
| `regresion-etapa3.log`, `regresion-etapa4.log`, `regresiones.json` | Resultados completos de las 101 regresiones |
| `ejecucion-reproducibilidad.json` | Salidas, códigos y SHA-256 de los tres programas |
| `controles-adicionales.json` | Fallos de interfaz, revisión de rutas y DLL importadas |

Para repetir las comprobaciones normales desde la raíz, en MSYS2 UCRT64:

```bash
bash compilar.sh
python -B pruebas/probar_etapa3.py
python -B pruebas/probar_etapa4.py
python -B salida_python/ejemplo1.py
python -B salida_python/ejemplo2.py
python -B salida_python/ejemplo3.py
```

Para comprobar reproducibilidad sin cambiar los archivos definitivos:

```bash
temporal=$(mktemp -d)
./compilador.exe ejemplos/ejemplo1.c "$temporal/ejemplo1.py"
./compilador.exe ejemplos/ejemplo2.c "$temporal/ejemplo2.py"
./compilador.exe ejemplos/ejemplo3.c "$temporal/ejemplo3.py"
cmp salida_python/ejemplo1.py "$temporal/ejemplo1.py"
cmp salida_python/ejemplo2.py "$temporal/ejemplo2.py"
cmp salida_python/ejemplo3.py "$temporal/ejemplo3.py"
```

Los `cmp` son comandos propuestos para repetir la comprobación; en esta
validación la igualdad se verificó mediante comparación directa de bytes
en Python. Los resultados informados corresponden a esas ejecuciones reales.

## Conclusión de la etapa

**Validación funcional aprobada dentro del alcance definido.** Las 101
regresiones continúan aprobadas; los tres originales tienen traducciones
válidas, equivalentes y reproducibles; la copia completa se compila desde
cero en una ruta con espacios sin depender de la ubicación original.

El proyecto está preparado para una demostración técnica con las herramientas
indicadas. Para la entrega definitiva quedan la actualización documental de
la etapa 7 y la preparación de una copia o paquete que incluya todos los
archivos validados, también los que Git todavía no sigue. Esto no equivale
a certificar un compilador completo de C ni portabilidad universal.

La etapa 6 termina aquí. No se inició la documentación definitiva de la
etapa 7 y no se realizaron commits ni push.
