# Etapa 7: documentación final y preparación de entrega

## Objetivo y alcance

Cerrar la documentación del transpilador de C a Python y preparar una
propuesta de entrega y publicación. El desarrollo funcional estaba
terminado. Esta etapa no cambia fuentes, gramática, materiales académicos,
pruebas previas ni traducciones definitivas.

Se revisaron `git status` y `.gitignore` antes de trabajar. Los cambios
previos de `compilar.sh`, `src/analizador_c.l` y `src/parser.y` pertenecen
a las etapas de implementación y se conservaron. No se ejecutaron
`git add`, `git commit` ni `git push`.

## Documentos entregados

| Documento | Trabajo realizado |
|---|---|
| [README.md](../README.md) | Actualizado: objetivos, alcance real, instalación, estructura, uso, pruebas y límites |
| [GRAMATICA.md](../GRAMATICA.md) | Actualizado: 21 no terminales, 31 tokens declarados, 45 alternativas, valores y acciones |
| [PRESENTACION.md](../PRESENTACION.md) | Actualizado: guion natural de exposición y preguntas con respuestas |
| [ARQUITECTURA.md](../ARQUITECTURA.md) | Creado: responsabilidades, flujo, símbolos, memoria y recorridos de instrucciones |
| [COMANDOS_DEMOSTRACION.txt](../COMANDOS_DEMOSTRACION.txt) | Creado: guía ejecutable con Bash para compilar, analizar, traducir, ejecutar y comparar |
| `pruebas/ETAPA7.md` | Creado: este registro de cierre y propuesta para GitHub |

La estructura completa se mantiene en el README para evitar duplicarla
aquí. La entrega queda organizada en las cinco fuentes de construcción
(cuatro archivos de `src/` y `compilar.sh`), tres C originales, tres
Python generados, pruebas e informes, documentación y referencias del
profesor. Los artefactos de compilación se pueden regenerar.

## Respaldo e integridad

Antes de editar se copiaron README, GRAMATICA y PRESENTACION a
`respaldo_documentos/`, dentro de un directorio externo al proyecto:

```text
%TEMP%\ProyectoProgramacionDeSistemas-etapa7-845dq8e8\
```

Esta notación usa el directorio temporal de Windows; no es una ruta
incorporada al programa ni un archivo que deba publicarse.

El manifiesto `sha256-protegidos-inicial.json` protege **41 archivos**:

| Grupo | Archivos |
|---|---:|
| Fuentes del transpilador y script de compilación | 5 |
| Programas originales C | 3 |
| Traducciones definitivas Python | 3 |
| PDF y archivos de los cinco ejemplos del profesor, incluido compilar.txt | 14 |
| Pruebas y los cuatro informes anteriores | 15 |
| .gitignore | 1 |
| **Total** | **41** |

Resultado final: **41/41 huellas SHA-256 idénticas**. Los tres documentos
actualizados se excluyeron del manifiesto porque su edición estaba
autorizada y cuentan con respaldo. También se excluyeron `.git/`,
cachés y los artefactos regenerables `compilador.exe`,
`src/parser.tab.c`, `src/parser.tab.h` y `src/lex.yy.c`.
Los tres documentos nuevos no existían al levantar el manifiesto.

## Entorno y compilación

Se seleccionaron explícitamente `MSYSTEM=UCRT64`, GCC de
`/ucrt64/bin` y las herramientas MSYS de `/usr/bin`. Se invocó el
Bash instalado con MSYS2 por su ruta completa desde Windows para evitar
resolver otro Bash. Las ejecuciones Python usaron `-B`.

| Herramienta | Versión observada |
|---|---|
| Bash | 5.3.15(1)-release |
| GNU Bison | 3.8.2 |
| Flex | 2.6.4 |
| GCC | 16.2.0, Rev3 de MSYS2 |
| Destino de GCC | x86_64-w64-mingw32 |
| Python UCRT64 | 3.14.7 |

Se ejecutó `bash compilar.sh` en el proyecto original y se compiló
desde cero una copia sin ejecutable ni fuentes generadas, situada en
`copia documental con espacios/` dentro del directorio externo.
Ambas compilaciones terminaron con código 0, sin advertencias de
Bison, Flex o GCC y sin conflictos gramaticales. La compilación original
tuvo stderr vacío.

La instalación mediante pacman descrita en README se contrastó con
documentación oficial de MSYS2 y sus paquetes. No se realizó una
instalación nueva ni una actualización del sistema.

## Validación ejecutada

La guía de demostración se ejecutó completa en la copia temporal.
De este modo sus comandos de generación no reemplazaron las traducciones
definitivas del proyecto original.

| Comprobación | Esperado | Obtenido | Estado |
|---|---|---|---|
| Compilar proyecto original | Código 0, sin advertencias ni conflictos | Código 0; stderr vacío | Aprobado |
| Compilar copia con espacios | Generación desde las fuentes originales | Código 0; sin advertencias ni conflictos | Aprobado |
| `bash -n COMANDOS_DEMOSTRACION.txt` | Sintaxis Bash válida | Código 0 | Aprobado |
| Guía completa corregida | Finalizar todas sus secciones | Código 0 | Aprobado |
| Etapa 3 | 33 comprobaciones correctas | **33/33** | Aprobado |
| Etapa 4 | 68 comprobaciones correctas | **68/68** | Aprobado |
| Comparaciones internas de etapa 4 | 37 equivalencias C/Python | **37/37**, incluidas en las 68 pruebas | Aprobado |
| Análisis de los tres originales | Código 0 en cada caso | 3/3 | Aprobado |
| Traducción y sintaxis Python | Generar y aceptar los tres archivos | 3/3; `ast.parse` correcto | Aprobado |
| Comparación adicional de los tres originales | stdout, stderr y terminación iguales | 3/3; bytes idénticos y código 0 | Aprobado |
| Reproducibilidad | Regenerados iguales a los definitivos | 3/3 idénticos byte a byte | Aprobado |
| Errores de demostración | Código 1; no publicar salida semántica inválida | Error léxico, sintáctico y semántico detectados | Aprobado |
| Gramática documentada | Misma secuencia de producciones que parser.y | 21 no terminales y 45 alternativas coincidentes | Aprobado |
| Terminales documentados | Todos los tokens declarados | 31/31 presentes | Aprobado |
| Integridad de archivos protegidos | SHA-256 sin cambios | 41/41 | Aprobado |

Las 101 pruebas son las baterías existentes, no 101 casos nuevos.
Las tres comparaciones adicionales son la demostración explícita de
los programas originales; no se suman a las 37 comparaciones internas.
En la comprobación de producciones se retiraron las acciones C de
ambos textos antes de comparar; las dos acciones intermedias se
documentan por separado porque afectan a los índices de Bison.

Los enlaces locales de la documentación se comprobaron contra archivos
existentes. Los comandos de instalación y las acciones futuras de
publicación no forman parte de los comandos ejecutados.

### Resultados visibles

Los siguientes bloques muestran el texto de stdout; las capturas de
C y Python también coincidieron en los saltos de línea de Windows.
No hubo contenido en stderr y ambos lenguajes devolvieron 0.

| Ejemplo | stdout |
|---|---|
| 1 | `La suma es: 13` + salto; `La resta es: 3` + salto; `La multiplicacion es: 40` + salto |
| 2 | `El promedio es: 8.33` + salto; `El alumno aprobo` + salto |
| 3 | `El numero es negativo` + salto |

### Incidencia de la guía

El primer recorrido aprobó las 101 regresiones, pero se detuvo con
código 127 al encontrar `cmp` ausente en esta instalación.
Se corrigió únicamente la guía nueva para comparar bytes mediante
la biblioteca estándar de Python, sin añadir dependencias.
Después se ejecutó nuevamente la guía completa: código 0 y
101/101 pruebas correctas. No se contabiliza el primer recorrido
como una demostración completa aprobada. El transpilador no cambió.

Los comandos `cmp` que aparecen en el informe histórico de etapa 6
eran propuestas para repetir la comparación, tal como aclara ese
informe. La guía actual sustituye esa dependencia; el informe previo
se conserva intacto.

## Matriz de cumplimiento académico

El PDF local `04 05 PS Construcción de un compilador (1).pdf` y los
cinco ejemplos son las referencias académicas. La matriz distingue
sus conceptos de las adaptaciones solicitadas para este proyecto.

| Elemento | Implementación y archivos | Referencia y evidencia | Estado |
|---|---|---|---|
| Analizador léxico | Reglas y estados en `src/analizador_c.l` | PDF y ejemplos 01–05: Flex y tokens; etapa 3 verifica cadenas, comentarios, NUL y caracteres inválidos | Cumple |
| Analizador sintáctico | `yyparse`, `yyerror`, ubicaciones en `src/parser.y` | PDF y cinco parsers; compilación sin conflictos y rechazo de entradas inválidas | Cumple |
| Gramática | 21 no terminales y 45 alternativas en `src/parser.y` | PDF: producciones de Bison; correspondencia automática con GRAMATICA.md | Cumple el subconjunto |
| Valores semánticos | `yytext`, `yylval`, `%union`, `%token`, `%type`, `$n` y `$$` | PDF y calculadoras 01–02; tipos compartidos en traduccion.h y regresión de etapa 3 | Cumple |
| Acciones semánticas | Acciones de declaraciones, expresiones, condicionales, printf y return | PDF y generadores 03–05; funciones reales descritas en GRAMATICA.md | Cumple |
| Tabla de símbolos | Arreglo de 100 entradas, búsqueda, ámbitos e inicialización en `src/traduccion.c` | Ejemplos 02 y 05 como referencia de tabla; ámbitos y análisis de ramas son adaptaciones del proyecto | Cumple |
| Generación automática | Fragmentos por reducción y `publicar_programa()` con `FILE *archivo_salida` y `fprintf()` | Ejemplos 03–05; archivo temporal y reemplazo son adaptaciones para impedir salidas parciales | Cumple |
| Traducción de C a Python | Tipos, operadores, bloques, impresión y retorno en `src/traduccion.c` | Destino Python solicitado por el proyecto; pruebas con fuentes y nombres adicionales | Cumple el alcance |
| Ejecución equivalente de los tres ejemplos | `ejemplos/ejemplo*.c` y `salida_python/ejemplo*.py` | Comparaciones GCC/Python, sintaxis y reproducibilidad verificadas de nuevo | Cumple |
| Detección y manejo de errores | Contadores léxicos, sintácticos, semánticos y comprobaciones de E/S | Ejemplos académicos como base de diagnósticos; regresiones verifican código 1 y conservación de destino | Cumple los casos cubiertos |

No se atribuyen al profesor como requisitos propios la salida Python,
el auxiliar float32, el análisis de inicialización o la publicación
mediante temporal. El ejemplo 05 calcula valores anticipadamente;
esta implementación conserva las expresiones, según lo solicitado.
Las pruebas aportan evidencia de los casos ejecutados, no una prueba
formal de equivalencia para todo C.

## Propuesta para GitHub

### Estado observado

| Grupo | Estado en Git y decisión propuesta |
|---|---|
| README.md, GRAMATICA.md, PRESENTACION.md | Ya registrados; actualizados en esta etapa |
| compilar.sh, src/analizador_c.l, src/parser.y | Registrados con modificaciones previas aprobadas; deben publicarse en su versión actual validada |
| src/traduccion.h y src/traduccion.c | Nuevos sin seguimiento; **indispensables para compilar** |
| ARQUITECTURA.md, COMANDOS_DEMOSTRACION.txt, pruebas/ETAPA7.md | Nuevos sin seguimiento; incorporar como documentación final |
| pruebas/probar_etapa3.py y probar_etapa4.py | Nuevos sin seguimiento; incorporar para reproducir las 101 comprobaciones |
| Casos nuevos de pruebas y ETAPA3.md–ETAPA6.md | Sin seguimiento; conservar e incorporar tras revisar la exposición de datos locales |
| Tres C originales y casos iniciales | Ya registrados; conservar sin cambios |
| salida_python/ejemplo1.py, ejemplo2.py, ejemplo3.py | Sin seguimiento; conservar como resultados generados de referencia |
| PDF, cinco carpetas del profesor y ejemplos/compilar.txt | Sin seguimiento; conservar localmente; decidir su distribución antes de hacer público el repositorio |

La raíz actual de Git todavía corresponde a una versión incompleta:
clonar solo los archivos registrados no incorpora `traduccion.c/.h`
ni las nuevas pruebas. Para reproducir el proyecto se necesita incluir
las cuatro fuentes de `src/`, `compilar.sh`, todos los casos usados
por las dos baterías, los tres C originales y la documentación.
Los tres Python permiten comprobar los resultados conservados, aunque
el transpilador puede regenerarlos.

El PDF y los ejemplos del profesor no son dependencias de construcción.
Su conservación local no implica autorización para redistribuirlos.
Se propone confirmar su inclusión y reconocimiento de autoría antes
de una publicación pública; no se añadió una licencia que les atribuya
permisos no indicados.

### Archivos que conviene excluir y finales de línea

La `.gitignore` actual ya excluye ejecutables, objetos,
`src/parser.tab.c`, `src/parser.tab.h`, `src/lex.yy.c`, `*.tmp`,
volcados y carpetas de editor. Se comprobó con `git check-ignore`.
No se modificó.

Antes de publicar, se propone añadir estas exclusiones:

```gitignore
__pycache__/
*.py[cod]
*.tmp.*
```

El último patrón cubre el nombre de temporales de publicación
`salida.py.tmp.PID.N`, que `*.tmp` no cubre. Las copias de respaldo,
logs, ejecutables C de prueba y capturas deben continuar fuera del
repositorio.

En este equipo `git config --get core.autocrlf` devuelve `true`.
Git advierte que podrá convertir LF a CRLF. No es una advertencia del
compilador. Para proteger scripts Bash en futuros clones se propone
aprobar una `.gitattributes` con:

```gitattributes
*.sh text eol=lf
COMANDOS_DEMOSTRACION.txt text eol=lf
```

Esa política todavía no se creó ni se aplicó al índice. La prueba de
ruta con espacios utiliza una copia de trabajo, no un clon de un
commit futuro. La verificación de ese clon debe hacerse después de
aprobar la preparación de publicación.

### Rutas, datos y revisión de privacidad

En las diez fuentes operativas revisadas (cuatro fuentes C/Flex/Bison,
script, dos baterías y tres Python) no se detectaron rutas absolutas
de usuario incorporadas al funcionamiento. La demostración se sitúa
en la carpeta del propio archivo, sin una ruta personal fija.

Los informes históricos ETAPA3–ETAPA6 contienen rutas del equipo,
nombre del perfil Windows y directorios temporales; ETAPA4 y ETAPA5
también documentan una alternativa local de Python. Son evidencia de
aquellas ejecuciones, no requisitos de instalación. No se editaron
porque deben conservarse. Antes de publicar se propone revisar con
el usuario si se mantienen o se prepara una copia pública anonimizada
sin alterar el respaldo histórico.

Una búsqueda de patrones comunes de claves privadas y tokens en
archivos de texto no encontró coincidencias. Esto no certifica la
ausencia de cualquier dato sensible ni inspecciona el historial Git.
No se publican respaldos ni logs locales de esta validación.

### Secuencia propuesta, pendiente de autorización

1. Aprobar el conjunto de archivos, el tratamiento de los informes
   históricos y la distribución de los materiales del profesor.
2. Aprobar las exclusiones y la política LF propuestas.
3. Revisar un diff final y preparar el registro de todos los archivos
   esenciales de la versión validada.
4. Probar un clon del commit preparado antes de realizar el push.
5. Publicar únicamente después de la autorización correspondiente.

Esta es una propuesta; no se ejecutó ninguna de esas operaciones
de preparación del índice, commit o publicación.

## Evidencia conservada fuera del proyecto

Dentro del directorio temporal indicado al comienzo se guardaron:

| Evidencia | Contenido |
|---|---|
| `respaldo_documentos/` | Versiones previas de los tres documentos actualizados |
| `git-status-inicial.txt` | Estado anterior al trabajo documental |
| `sha256-protegidos-inicial.json` | Manifiesto de los 41 archivos protegidos |
| `compilacion-original.stdout.txt` y `.stderr.txt` | Compilación del proyecto original |
| `demostracion-completa.stdout.txt` y `.stderr.txt` | Primer recorrido, incluido el fallo por ausencia de cmp |
| `demostracion-final.stdout.txt` y `.stderr.txt` | Recorrido completo corregido y las 101 regresiones |
| `revision-documental.json`, `validacion-final.json` | Correspondencia de gramática y resultados automáticos |
| `integridad-final.json` | Comparación final del manifiesto y respaldos |
| `copia documental con espacios/` | Copia usada para ejecutar la guía sin modificar los originales |

Los diagnósticos intencionales de errores en la demostración aparecen
en stderr; sus códigos 1 fueron comprobados. La guía completa devuelve 0
porque verifica esos rechazos como resultados esperados.

## Limitaciones y conclusión

Permanecen los límites de [README.md](../README.md): subconjunto fijo,
100 declaraciones totales, inicialización conservadora, formatos y
escapes restringidos, perfil numérico finito dentro de rango y
publicación específica de Windows. No se amplió la gramática ni se
corrigió código fuente durante esta etapa.

La documentación y la demostración local están preparadas para la
entrega y exposición. La compilación, las 101 regresiones y los tres
resultados originales se validaron otra vez. No se encontró un defecto
del transpilador que requiera modificar sus fuentes. Quedan decisiones
de preparación del repositorio antes de publicar una versión que otros
puedan clonar con todos los archivos necesarios.

La etapa 7 termina con esta entrega. **No se publicó en GitHub; se espera
autorización para continuar con la preparación y publicación.**
