# Transpilador de un subconjunto de C a Python

Proyecto universitario de **Programación de Sistemas**. Flex reconoce los
lexemas, Bison valida la gramática y sus acciones semánticas construyen código
Python. El programa está escrito en C y se compila con GCC en Windows,
utilizando MSYS2 UCRT64.

El objetivo general es traducir automáticamente los tres programas originales
de [ejemplos/](ejemplos/) y conservar sus resultados al ejecutarlos. Los
objetivos específicos son comunicar valores entre Flex y Bison, comprobar
declaraciones y tipos, gestionar ámbitos, generar código mediante
`FILE *archivo_salida` y `fprintf()`, y contrastar las ejecuciones de C y Python.
Las expresiones se calculan al ejecutar Python; el transpilador no realiza
*constant folding*.

## Alcance

Reconoce exactamente un `#include <stdio.h>` seguido de `int main() { ... }`:

- Declaraciones individuales `int` y `float`, con inicialización opcional;
  asignaciones, `+`, `-`, `*`, `/`, paréntesis y menos unario.
- `if`, `else if` y `else` con bloques entre llaves. Cada condición contiene
  una comparación: `>`, `<`, `>=`, `<=`, `==` o `!=`.
- `printf` con cadena inicial y expresiones numéricas opcionales, y
  `return expresion;`.
- Comentarios `//` y `/* ... */`, que se descartan.

La modalidad de traducción comprueba declaraciones, usos, inicialización,
tipos y formatos de impresión. La modalidad original de análisis comprueba
únicamente léxico y sintaxis; que acepte un programa no garantiza que se
pueda traducir.

## Herramientas e instalación

Se necesita Windows, MSYS2 UCRT64, Bash, Flex, GNU Bison, GCC y Python 3.
Las pruebas utilizan solamente la biblioteca estándar de Python. Git es
opcional para compilar una copia descargada.

Abra la terminal **MSYS2 UCRT64**, cuyo entorno coloca `/ucrt64/bin`
antes de `/usr/bin`. Véase la [documentación de entornos de MSYS2](https://www.msys2.org/docs/environments/).
Para preparar un equipo nuevo:

```bash
pacman -Suy
# Si se solicita cerrar la terminal, reabrir UCRT64 y repetir la actualización.
pacman -S --needed bison flex mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-python
# Opcional, si se necesita Git y todavía no está disponible:
pacman -S --needed git
```

Referencias oficiales: [actualización de MSYS2](https://www.msys2.org/docs/updating/),
paquetes [Bison](https://packages.msys2.org/packages/bison),
[Flex](https://packages.msys2.org/packages/flex),
[GCC UCRT64](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-gcc)
y [Python UCRT64](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-python).
Bison y Flex proceden del entorno MSYS; GCC y Python, de UCRT64.
Estos comandos de instalación son instrucciones para otro equipo; no se
instalaron ni actualizaron paquetes durante el cierre documental.

Versiones utilizadas en la validación: Bash 5.3.15, Bison 3.8.2, Flex 2.6.4,
GCC 16.2.0 (Rev3) y Python 3.14.7. No constituyen una matriz de versiones mínimas.

## Estructura del proyecto

```text
ProyectoProgramacionDeSistemas/
├── README.md
├── GRAMATICA.md
├── PRESENTACION.md
├── ARQUITECTURA.md
├── COMANDOS_DEMOSTRACION.txt
├── .gitattributes
├── .gitignore
├── compilar.sh
├── src/
│   ├── analizador_c.l
│   ├── parser.y
│   ├── traduccion.h
│   └── traduccion.c
├── ejemplos/
│   ├── ejemplo1.c
│   ├── ejemplo2.c
│   └── ejemplo3.c
├── salida_python/
│   ├── ejemplo1.py
│   ├── ejemplo2.py
│   └── ejemplo3.py
└── pruebas/
    ├── probar_etapa3.py
    ├── probar_etapa4.py
    ├── valores_semanticos.c
    ├── valido_comentarios.c
    ├── valido_cadenas_escapadas.c
    ├── invalido_parentesis.c
    ├── invalido_sin_punto_y_coma.c
    ├── invalido_cadena_sin_cerrar.c
    ├── invalido_caracter_entre_printf.c
    ├── invalido_comentario_sin_cerrar.c
    ├── invalido_texto_entre_cadenas.c
    └── ETAPA3.md ... ETAPA7.md
```

Al compilar aparecen `compilador.exe`, `src/parser.tab.c`,
`src/parser.tab.h` y `src/lex.yy.c`; son artefactos regenerables excluidos
de Git. Los tres Python se conservan como resultados de referencia y se
pueden volver a generar. El repositorio contiene únicamente archivos del
proyecto. El PDF y los cinco ejemplos del profesor se conservan fuera de
la distribución y sus rutas están excluidas explícitamente en `.gitignore`;
no son necesarios para compilar ni ejecutar las pruebas.

| Archivo | Responsabilidad |
|---|---|
| `src/analizador_c.l` | Tokens, copia de lexemas y ubicación de errores léxicos |
| `src/parser.y` | Producciones, acciones semánticas y modalidades de ejecución |
| `src/traduccion.h` | Tipos semánticos y funciones compartidas |
| `src/traduccion.c` | Símbolos, tipos, expresiones, bloques y publicación de Python |
| `compilar.sh` | Ejecuta Bison, Flex y GCC desde la ubicación del propio script |
| `pruebas/probar_etapa3.py` | Regresiones léxicas, sintácticas y de valores semánticos |
| `pruebas/probar_etapa4.py` | Semántica, traducción, comparación y protección de salida |

## Compilación y uso

Desde la raíz del proyecto, en MSYS2 UCRT64:

```bash
echo "$MSYSTEM"                     # Debe mostrar UCRT64.
command -v bash bison flex gcc python
bash compilar.sh
```

El script ejecuta Bison con `-Wall` y trata como errores los conflictos
shift/reduce y reduce/reduce; GCC utiliza `-Wall -Wextra`.

Análisis léxico y sintáctico, sin archivo de salida:

```bash
./compilador.exe ejemplos/ejemplo1.c
./compilador.exe ejemplos/ejemplo2.c
./compilador.exe ejemplos/ejemplo3.c
```

También se admite la entrada estándar si no se proporciona un archivo.
Un resultado correcto devuelve 0; errores de análisis, traducción o E/S
devuelven 1.

Traducción automática y ejecución:

```bash
mkdir -p salida_python
./compilador.exe ejemplos/ejemplo1.c salida_python/ejemplo1.py
./compilador.exe ejemplos/ejemplo2.c salida_python/ejemplo2.py
./compilador.exe ejemplos/ejemplo3.c salida_python/ejemplo3.py
python -B salida_python/ejemplo1.py
python -B salida_python/ejemplo2.py
python -B salida_python/ejemplo3.py
```

El directorio de destino debe existir. Si hay un error léxico, sintáctico o
semántico, no se publica una nueva salida. Tras validar, se escribe un
temporal mediante `fprintf()` y se reemplaza el destino únicamente si la
escritura y el cierre tienen éxito. Una salida anterior se conserva ante
un fallo. No se permite utilizar el mismo archivo como entrada y salida.

Flujo: archivo C → Flex → tokens y valores → Bison y acciones →
tabla de símbolos y fragmentos Python → validación → archivo Python →
ejecución con Python 3. El ejecutable no ejecuta los programas recibidos.

## Ejemplos y pruebas

| Programa | Resultado esperado al ejecutarlo | Terminación C / Python |
|---|---|---|
| `ejemplo1` | Suma 13, resta 3, multiplicación 40, cada resultado en su línea | 0 / 0 |
| `ejemplo2` | `El promedio es: 8.33` y `El alumno aprobo` | 0 / 0 |
| `ejemplo3` | `El numero es negativo` | 0 / 0 |

Para repetir las regresiones:

```bash
python -B pruebas/probar_etapa3.py
python -B pruebas/probar_etapa4.py
```

Son **33 + 68 = 101 comprobaciones**. La segunda batería incluye
37 comparaciones entre C compilado y Python generado, casos semánticos
inválidos y conservación de archivos ante errores. `valores_semanticos.c`
es un programa auxiliar de prueba de las funciones C, no una entrada
del subconjunto que acepte el transpilador.

La [guía ejecutable de demostración](COMANDOS_DEMOSTRACION.txt) reúne los
comandos, errores esperados y comparaciones adicionales. Puede ejecutarse
desde UCRT64 con `bash COMANDOS_DEMOSTRACION.txt`; vuelve a generar las
tres traducciones automáticamente. El cierre y sus evidencias se registran
en [pruebas/ETAPA7.md](pruebas/ETAPA7.md), y la validación de portabilidad
en [pruebas/ETAPA6.md](pruebas/ETAPA6.md).

## Limitaciones

- No se implementa C completo: no hay ciclos, arreglos, otras funciones,
  punteros, operadores lógicos, incrementos ni declaraciones múltiples.
  Los bloques deben contener al menos una sentencia; un bloque que solo
  declara variables sin inicializar genera `pass`.
- La tabla admite 100 declaraciones en total, incluidas las de ámbitos
  ya cerrados. El análisis de inicialización es conservador: combina ramas
  alcanzables, pero no demuestra condiciones constantes ni relaciones
  entre diferentes `if`.
- Se admiten `%d`, `%f`, `%.Nf` y `%%`; los argumentos deben tener los
  tipos y la cantidad esperados. Los escapes traducibles son `\n`,
  `\t`, `\r`, `\\` y `\"`. No se admite `%s` ni concatenación de
  cadenas C. Los reales se escriben con punto, sin exponentes ni sufijos.
- La división entera usa un auxiliar que trunca hacia cero. Las conversiones
  a `float` y las operaciones de ese tipo usan redondeo binario de 32 bits.
  Se trabaja con valores finitos y dentro de rango: no se garantiza
  equivalencia para desbordamientos, división por cero o todos los casos
  extremos de redondeo. No se analizan todos los errores de ejecución.
- La publicación utiliza API de Windows. Se validó en MSYS2 UCRT64,
  incluida otra ruta con espacios; no se promete compatibilidad con
  otros sistemas operativos ni con todas las rutas Unicode.
  Los códigos de terminación comparados son ordinarios y pequeños.

## Material académico y cierre

El PDF y los cinco ejemplos del profesor sirven como referencia para
Flex/Bison, valores semánticos, tabla de símbolos y escritura con
`fprintf()`. La adaptación a Python, la gestión de inicialización y la
publicación segura responden a este proyecto. El cálculo anticipado del
ejemplo 05 del profesor no se trasladó a esta implementación.

El proyecto cumple el objetivo de generar automáticamente las tres
traducciones y contrastar sus ejecuciones dentro del alcance descrito.
Para estudiar su funcionamiento consulte [GRAMATICA.md](GRAMATICA.md),
[ARQUITECTURA.md](ARQUITECTURA.md) y [PRESENTACION.md](PRESENTACION.md).
Los informes de las etapas describen el estado del proyecto durante cada
validación; sus referencias a tareas pendientes corresponden a esos
momentos. Las copias públicas de los informes históricos omiten las rutas
personales del equipo utilizado.
