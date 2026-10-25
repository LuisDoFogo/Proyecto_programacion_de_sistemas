# Gramática del subconjunto de C

## Criterios

La gramática es libre de contexto: cada producción tiene un único no terminal a la izquierda. La recursión por la izquierda es intencional en `lista_sentencias`, `lista_argumentos`, `expresion` y `termino`. Bison puede procesar estas producciones. La separación entre expresiones aditivas, multiplicativas y factores establece la precedencia aritmética. La precedencia `IF_WITHOUT_ELSE`/`ELSE` asocia `else` con el `if` más cercano.

**Importante:** la recursión por la izquierda no es directamente adecuada para un analizador predictivo LL(1) sin transformar la gramática. Este proyecto utiliza Bison para ejecutar el análisis sintáctico; FIRST/FOLLOW y LL(1) pueden estudiarse por separado sobre una versión transformada si el curso lo solicita.

## Producciones

```text
programa                 -> directiva_inclusion definicion_funcion
directiva_inclusion      -> INCLUDE HEADER
definicion_funcion       -> INT MAIN LPAREN RPAREN bloque
bloque                   -> LBRACE lista_sentencias RBRACE
lista_sentencias         -> lista_sentencias sentencia | sentencia
sentencia                -> declaracion | asignacion | llamada_printf | sentencia_if | sentencia_return
declaracion              -> tipo ID SEMICOLON | tipo ID ASSIGN expresion SEMICOLON
tipo                     -> INT | FLOAT
asignacion               -> ID ASSIGN expresion SEMICOLON
llamada_printf           -> PRINTF LPAREN STRING RPAREN SEMICOLON
                         | PRINTF LPAREN STRING COMMA lista_argumentos RPAREN SEMICOLON
lista_argumentos         -> lista_argumentos COMMA expresion | expresion
sentencia_if             -> IF LPAREN condicion RPAREN bloque
                         | IF LPAREN condicion RPAREN bloque ELSE sentencia
condicion                -> expresion operador_relacional expresion
operador_relacional      -> GE | LE | EQ | NE | GT | LT
sentencia_return         -> RETURN expresion SEMICOLON
expresion                -> expresion PLUS termino | expresion MINUS termino | termino
termino                  -> termino MULT factor | termino DIV factor | factor
factor                   -> ID | INT_LITERAL | FLOAT_LITERAL
                         | LPAREN expresion RPAREN | MINUS factor
```

## Precedencia

- Paréntesis agrupan expresiones.
- `*` y `/` tienen mayor precedencia que `+` y `-` por la estructura de las producciones.
- La recursión izquierda hace que `a - b - c` se agrupe como `(a - b) - c`.
- Se permite menos unario, como en `-3`.
- Las condiciones admiten `>=`, `<=`, `==`, `!=`, `>` y `<`.
- El `else` se asocia al `if` más cercano.

## Tokens principales

| Token | Significado |
|---|---|
| `INCLUDE`, `HEADER` | `#include <stdio.h>` |
| `INT`, `FLOAT`, `MAIN`, `IF`, `ELSE`, `RETURN`, `PRINTF` | Palabras reservadas |
| `ID` | Identificador |
| `INT_LITERAL`, `FLOAT_LITERAL` | Literales numéricos |
| `STRING` | Cadena entre comillas |
| `GE`, `LE`, `EQ`, `NE`, `GT`, `LT` | Operadores relacionales |
| `ASSIGN`, `PLUS`, `MINUS`, `MULT`, `DIV` | Asignación y operadores aritméticos |
| `LPAREN`, `RPAREN`, `LBRACE`, `RBRACE` | Delimitadores |
| `SEMICOLON`, `COMMA` | Separadores |

## Restricciones

La gramática requiere `#include <stdio.h>` y `int main()`. Cada bloque debe contener al menos una sentencia. Solo reconoce declaraciones `int`/`float`, asignaciones, `printf`, `if`/`else if`/`else` y `return`. No comprueba si las variables existen o si sus tipos son correctos; eso sería análisis semántico.
