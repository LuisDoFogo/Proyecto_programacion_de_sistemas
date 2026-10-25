# Guion breve para demostrar el analizador sintáctico

## 1. Explicar el objetivo

El analizador léxico reconoce los tokens. El analizador sintáctico recibe esos tokens y verifica que estén organizados de acuerdo con la gramática del subconjunto de C que se definió para los tres ejemplos.

## 2. Mostrar la gramática

Abrir `GRAMATICA.md`. Explicar que cada producción tiene un no terminal a la izquierda, por lo que la gramática es libre de contexto. Mostrar las producciones recursivas por la izquierda de `expresion`, `termino` y `lista_sentencias`. Explicar que los niveles separados de expresiones establecen la precedencia de los operadores.

## 3. Compilar

Desde MSYS2 UCRT64, en la carpeta del proyecto, ejecutar:

```bash
bash compilar.sh
```

## 4. Ejecutar programas correctos

```bash
./compilador.exe ejemplos/ejemplo1.c
./compilador.exe ejemplos/ejemplo2.c
./compilador.exe ejemplos/ejemplo3.c
```

Deben finalizar con el mensaje de análisis sintáctico correcto.

## 5. Demostrar detección de error

```bash
./compilador.exe pruebas/error_sintaxis.c
```

La prueba tiene una declaración sin punto y coma. El analizador debe reportar un error sintáctico.

## 6. Demostrar que los comentarios se ignoran

```bash
./compilador.exe pruebas/comentarios_ignorados.c
```

Los comentarios no se entregan al analizador sintáctico como tokens. El texto `//` dentro de una cadena debe conservarse como parte de la cadena.

## Explicación breve para el profesor

“Primero Flex convierte el archivo fuente en tokens y descarta los comentarios. Después Bison comprueba la secuencia de tokens con una gramática libre de contexto. La gramática está organizada para dar prioridad a multiplicación y división sobre suma y resta. También tiene producciones recursivas por la izquierda para listas y expresiones. El proyecto reconoce el subconjunto usado en los tres ejemplos y reporta cuando encuentra una estructura sintácticamente incorrecta.”
