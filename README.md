# Analizador léxico y sintáctico de un subconjunto de C

Proyecto académico de Programación de Sistemas con Flex, GNU Bison y GCC. El programa analiza la sintaxis de una estructura limitada de C; **no realiza análisis semántico** ni ejecuta el código recibido.

## Qué reconoce

- `#include <stdio.h>` y la función `int main() { ... }`.
- Declaraciones y asignaciones de variables `int` y `float`.
- Expresiones con `+`, `-`, `*`, `/`, paréntesis y menos unario.
- Comparaciones `>`, `<`, `>=`, `<=`, `==` y `!=` en condiciones `if`, `else if` y `else`.
- Llamadas a `printf` con cadena y argumentos opcionales, y sentencias `return`.
- Comentarios de línea y de bloque.

No es un compilador de C completo: no admite, por ejemplo, ciclos, arreglos ni funciones definidas por el usuario, y no comprueba tipos ni declaraciones.

## Archivos

- `src/analizador_c.l`: reglas léxicas de Flex.
- `src/parser.y`: gramática de Bison.
- `ejemplos/`: tres programas válidos.
- `pruebas/`: un caso válido con comentarios y dos casos inválidos.
- `GRAMATICA.md`: producciones y explicación de la gramática.

## Requisitos e instalación

En Windows, instala [MSYS2](https://www.msys2.org/), abre la terminal **MSYS2 UCRT64** y ejecuta:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-flex mingw-w64-ucrt-x86_64-bison
```

También se requiere Git para obtener el proyecto. Flex, Bison y GCC deben estar disponibles en `PATH` dentro de la terminal UCRT64.

## Compilación

Desde la raíz del proyecto:

```bash
bash compilar.sh
```

La compilación genera `compilador.exe` y archivos intermedios bajo `src/`; esos artefactos están excluidos de Git.

## Ejecución y pruebas

```bash
./compilador.exe ejemplos/ejemplo1.c
./compilador.exe ejemplos/ejemplo2.c
./compilador.exe ejemplos/ejemplo3.c
./compilador.exe pruebas/valido_comentarios.c
./compilador.exe pruebas/invalido_sin_punto_y_coma.c
./compilador.exe pruebas/invalido_parentesis.c
```

Los archivos válidos deben mostrar `Analisis sintactico correcto` y devolver código 0. Los dos casos inválidos deben reportar `Error sintactico` y devolver un código distinto de 0.

## Documentación

Consulta [GRAMATICA.md](GRAMATICA.md) para ver las producciones completas y las decisiones de precedencia.
