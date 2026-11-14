# Gramática y traducción implementadas

La referencia de este documento es [src/parser.y](src/parser.y);
los patrones léxicos están en [src/analizador_c.l](src/analizador_c.l).
El símbolo inicial es **programa**, declarado con `%start programa`.
Bison utiliza `%locations` y `%define parse.error verbose`.

## Terminales y reconocimiento léxico

| Tokens | Texto reconocido por Flex |
|---|---|
| `INCLUDE HEADER` | `#include`, `<stdio.h>` |
| `INT FLOAT MAIN IF ELSE RETURN PRINTF` | `int float main if else return printf` |
| `ID` | `[a-zA-Z_][a-zA-Z0-9_]*`, salvo palabras anteriores |
| `INT_LITERAL` | Una o más cifras; en traducción se valida con base C y rango de int |
| `FLOAT_LITERAL` | `[0-9]+\.[0-9]*` o `[0-9]*\.[0-9]+` |
| `STRING` | Cadena entre comillas dobles, con escapes, sin salto físico de línea ni NUL |
| `GE LE EQ NE GT LT` | `>= <= == != > <` |
| `ASSIGN PLUS MINUS MULT DIV` | `= + - * /` |
| `LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA` | `( ) { } ; ,` |
| `ERROR_LEXICO` | Token de fallo; ninguna producción lo admite |

Flex devuelve 0 al terminar la entrada. Espacios y comentarios no son
terminales de la gramática. El estado exclusivo `COMENTARIO` detecta
comentarios de bloque sin cierre. Las cadenas sin cierre, los caracteres
inválidos y los bytes NUL incrementan el contador léxico y producen fallo.
La expresión regular de cadena evita unir dos cadenas distintas y permite
comillas escapadas. Que un escape sea reconocido léxicamente no significa
que la modalidad de traducción lo admita.

## Producciones completas

Los nombres que aparecen a la izquierda son todos los no terminales.
La notación `%empty` representa la alternativa vacía. Se muestran las dos
acciones intermedias porque afectan al orden del análisis y a los índices
de los valores; las acciones finales se explican después.

```bison
programa
    : directiva_inclusion definicion_funcion
    ;
directiva_inclusion
    : INCLUDE HEADER
    ;
definicion_funcion
    : INT MAIN LPAREN RPAREN bloque
    ;
bloque
    : LBRACE { entrar_ambito(); } lista_sentencias RBRACE
    ;
lista_sentencias
    : lista_sentencias sentencia
    | sentencia
    ;
sentencia
    : declaracion
    | asignacion
    | llamada_printf
    | sentencia_if
    | sentencia_return
    ;
declaracion
    : cabecera_declaracion SEMICOLON
    | cabecera_declaracion ASSIGN expresion SEMICOLON
    ;
cabecera_declaracion
    : tipo ID
    ;
tipo
    : INT
    | FLOAT
    ;
asignacion
    : ID ASSIGN expresion SEMICOLON
    ;
llamada_printf
    : PRINTF LPAREN STRING RPAREN SEMICOLON
    | PRINTF LPAREN STRING COMMA lista_argumentos RPAREN SEMICOLON
    ;
lista_argumentos
    : lista_argumentos COMMA expresion
    | expresion
    ;
sentencia_if
    : inicio_if bloque { preparar_else($1); } parte_else
    ;
inicio_if
    : IF LPAREN condicion RPAREN
    ;
parte_else
    : %empty
    | ELSE bloque
    | ELSE sentencia_if
    ;
condicion
    : expresion operador_relacional expresion
    ;
operador_relacional
    : GE
    | LE
    | EQ
    | NE
    | GT
    | LT
    ;
sentencia_return
    : RETURN expresion SEMICOLON
    ;
expresion
    : expresion PLUS termino
    | expresion MINUS termino
    | termino
    ;
termino
    : termino MULT factor
    | termino DIV factor
    | factor
    ;
factor
    : ID
    | INT_LITERAL
    | FLOAT_LITERAL
    | LPAREN expresion RPAREN
    | MINUS factor
    ;
```

No hay producciones para bloques independientes, cuerpos vacíos,
declaraciones separadas por comas ni otras funciones. `int main(void)`
tampoco pertenece a esta gramática. `cabecera_declaracion`, `inicio_if`
y `parte_else` son auxiliares explícitos del archivo; las acciones
intermedias se convierten internamente en reglas auxiliares de Bison.

## Expresiones, precedencia y asociatividad

`factor` reconoce átomos, agrupación y menos unario; `termino` combina
multiplicación y división; `expresion` combina suma y resta. Por tanto,
el menos unario tiene mayor precedencia que `*` y `/`, y estos mayor
que `+` y `-`. La recursión por la izquierda hace que los operadores
binarios de cada nivel se asocien por la izquierda:
`a - b - c` se agrupa como `(a - b) - c`.

No se emplean declaraciones `%left`, `%right` o `%prec` en este parser.
La precedencia se establece por las producciones. Las comparaciones se
aceptan únicamente mediante `condicion`, entre dos expresiones aritméticas;
no se admiten comparaciones encadenadas ni operadores lógicos.

La estructura `Expresion` guarda texto, tipo y precedencia. Los
constructores conservan los paréntesis necesarios para reproducir la
agrupación; eliminan los explícitos redundantes. También conservan
agrupaciones a la derecha cuando podrían alterar operaciones de punto
flotante. No calculan el resultado numérico.

## Valores semánticos y memoria

`%union` contiene `char *texto`, `const char *operador`, `Tipo tipo`,
`Expresion expresion`, `Argumentos argumentos`, `int simbolo` y
`ContextoIf *contexto`.

| Símbolos tipados | Miembro de la unión y contenido |
|---|---|
| `ID INT_LITERAL FLOAT_LITERAL STRING` | `<texto>`: copia dinámica del lexema |
| `tipo` | `<tipo>`: `T_INT` o `T_FLOAT` |
| `operador_relacional` | `<operador>`: texto constante del operador Python |
| `expresion termino factor condicion` | `<expresion>`: texto, tipo y precedencia |
| `lista_argumentos` | `<argumentos>`: arreglo dinámico de expresiones |
| `cabecera_declaracion` | `<simbolo>`: índice de la variable declarada |
| `inicio_if` | `<contexto>`: condición y estados de inicialización |
| `definicion_funcion bloque lista_sentencias sentencia declaracion asignacion llamada_printf sentencia_if parte_else sentencia_return` | `<texto>`: fragmento Python con saltos de línea |

`programa` y `directiva_inclusion` no tienen `%type`; los demás tokens no
necesitan transportar datos. `T_DOUBLE` es interno: representa los
literales reales sin sufijo de C; no agrega declaraciones `double`.

Flex copia `yytext` mediante `copiar_lexema()` y coloca la copia en
`yylval.texto`. Bison recibe ese valor con el token. En
`expresion PLUS termino`, `$1` y `$3` son operandos, `$2` es el token
del operador sin dato asociado y `$$` recibe el resultado de
`crear_binaria($1, "+", $3, PREC_SUMA)`.

Los constructores consumen la propiedad de sus operandos y lexemas.
Las acciones liberan los identificadores después de registrarlos o
buscarlos. Hay `%destructor` para texto, expresiones, argumentos y
contextos de `if` que Bison descarte al interrumpir el análisis; las
reducciones normales gestionan su propia memoria. `@1`, `@2`, etc.
aportan línea y columna; el lexer actualiza `yylloc` en cada coincidencia.

## Acciones semánticas por producción

| Producción o grupo | Acción real |
|---|---|
| `programa`, `definicion_funcion` | Conservan el cuerpo construido; `main` decide si publicarlo |
| `bloque` | Entra al ámbito antes de leer sentencias, sale al cerrar y devuelve `$3` |
| `lista_sentencias`, `sentencia` | Concatenan mediante `unir_textos()` o propagan el valor |
| `tipo`, `cabecera_declaracion` | Determinan el tipo y llaman a `declarar_simbolo()` antes del inicializador |
| `declaracion` | Sin inicializador devuelve texto vacío; con él usa `generar_asignacion()` |
| `asignacion` | Busca el símbolo y genera asignación con conversión de tipo |
| `factor` | Usa `referencia_variable()`, `literal_numerico()`, `crear_negacion()` o propaga `$2` |
| `expresion`, `termino` | Usan `crear_binaria()` o propagan el operando |
| `condicion`, `operador_relacional` | Construyen la comparación con promoción de tipos |
| `lista_argumentos` | Usa `primer_argumento()` y `agregar_argumento()` |
| `llamada_printf` | `generar_printf()` valida cadena, formatos, cantidad y tipos |
| `inicio_if` | `iniciar_if()` guarda condición y estado de entrada |
| Acción intermedia de `sentencia_if` | `preparar_else($1)` guarda la primera rama y restaura la entrada |
| `parte_else`, `sentencia_if` | `generar_else()`, `generar_elif()` y `terminar_if()` generan y combinan ramas |
| `sentencia_return` | `generar_return()` convierte a int y marca fin del camino actual |

En `sentencia_if`, la acción intermedia cuenta como posición 3 y
`parte_else` es `$4`. El cierre llama a `terminar_if($1, $2, $4)`.
Esto evita confundir índices al explicar el código.

## Equivalencias de C a Python

Son ejemplos de fragmentos dentro de `main`; los auxiliares se emiten
solo cuando se usan. La indentación real es de cuatro espacios.

| Estructura | C | Python generado o tratamiento |
|---|---|---|
| Inclusión | `#include <stdio.h>` | Se valida; no genera import de stdio |
| Función principal | `int main() { return 0; }` | `def main():` y entrada mediante `raise SystemExit(main())` |
| Entero | `int x = 2;` | `x = 2` |
| Real | `float x = 2.5;` | `x = _c_f32(2.5)` |
| Sin inicialización | `int x;` | Solo registro semántico; no se inventa un valor |
| Asignación | `x = 3;` | `x = 3` si x es int |
| Aritmética entera | `x = 2 + 3 * 4;` | `x = 2 + 3 * 4` |
| División entera | `x = -7 / 3;` | `x = _c_idiv(-7, 3)`, resultado -2 al ejecutar |
| Menos unario | `x = -x;` | `x = -x` |
| Agrupación | `x = (2 + 3) * 4;` | `x = (2 + 3) * 4` |
| Relacionales | `if (x >= 2) { x = 1; }` | `if x >= 2:` seguido del bloque indentado |
| Alternativas | `else if (x < 0) { x = 0; }` / `else { x = 1; }` | `elif x < 0:` / `else:` y sus bloques |
| Impresión simple | `printf("Hola\n");` | `print("Hola\n", end="")` |
| Impresión numérica | `printf("%d\n", x);` | `print("%d\n" % (x,), end="")` |
| Retorno | `return x;` | `return x` o conversión a int si es necesaria |
| Bloque | `{ int x; }` | Sin sentencias emitidas se escribe `pass` |
| Comentarios | `// nota` o `/* nota */` | Se descartan, no se copian |

Los formatos admitidos son `%d`, `%f`, `%.Nf` y `%%`.
Las conversiones numéricas, ámbitos y errores se desarrollan en
[ARQUITECTURA.md](ARQUITECTURA.md); las restricciones de entrada están en
[README.md](README.md). El archivo solo se escribe después de que las
acciones y los contadores de errores permitan publicar el programa.
