# Guía para la exposición

## Guion sugerido

**Objetivo y alcance.** «Mi proyecto traduce automáticamente tres programas
de C a Python utilizando Flex, Bison y acciones semánticas escritas en C.
Trabaja con el subconjunto de nuestra gramática: declaraciones, asignaciones,
expresiones aritméticas, condicionales, printf y return».

**Qué construí.** «Un compilador transforma un programa a otro lenguaje,
que puede ser código máquina, intermedio o fuente. Un transpilador es un
compilador cuyo destino es otro lenguaje fuente. Un intérprete ejecuta
un programa. Aquí GCC compila el transpilador; después mi ejecutable
genera Python, y Python ejecuta el resultado».

**Flex.** «El archivo analizador_c.l contiene patrones para palabras,
identificadores, números, cadenas y signos. Flex genera lex.yy.c.
La función yylex devuelve un token por vez y descarta espacios y
comentarios. También conserva línea y columna para los errores».

**Bison.** «El archivo parser.y contiene una gramática libre de contexto:
cada producción tiene un no terminal a la izquierda. Bison genera
parser.tab.c y parser.tab.h. yyparse pide tokens a yylex y reduce las
producciones. La recursión de expresion, termino y factor define la
agrupación y precedencia».

**Comunicación y valores.** «yytext señala el lexema actual de Flex;
como cambia con cada lectura, copio los textos que necesito.
yylval transporta el valor asociado al token. %union define sus
posibles tipos; %token asocia tipos a tokens y %type a no terminales.
Por ejemplo, los identificadores llevan char * y las expresiones
llevan texto, tipo y precedencia».

Mostrar este fragmento real de `parser.y`:

```c
$$ = crear_binaria($1, "+", $3, PREC_SUMA);
```

«En expresion PLUS termino, $1 es la expresión izquierda, $2 es el token
PLUS y $3 es el término derecho. $$ es el resultado de la reducción.
No sumo los números en C: construyo el texto que Python evaluará.
Los paréntesis se conservan cuando hacen falta».

**Semántica.** «La tabla es un arreglo de hasta 100 declaraciones. Guarda
nombre C y Python, tipo, ámbito e inicialización. Verifico duplicados,
variables no declaradas, lecturas sin inicialización y formatos de printf.
Al abrir un bloque se entra al ámbito y al cerrarlo se desactivan sus
variables. En los if combino los caminos que pueden continuar».

**Generación.** «Las acciones van produciendo fragmentos Python.
Al terminar sin errores, publicar_programa abre un temporal mediante
FILE *archivo_salida y escribe con fprintf. Se incluyen solo los
auxiliares necesarios y se indenta con cuatro espacios. Si escritura
y cierre funcionan, se reemplaza el destino».

**Demostración.** «Compilo con bash compilar.sh en MSYS2 UCRT64.
Primero analizo los tres programas, luego los traduzco usando un segundo
argumento como salida. Muestro que los archivos se generan desde la
gramática y no están escritos como respuestas para ciertos nombres».

Seguir [COMANDOS_DEMOSTRACION.txt](COMANDOS_DEMOSTRACION.txt).
Puede estudiarse por secciones o ejecutarse completo con Bash.

**Resultados.** «El primer ejemplo conserva las operaciones y muestra 13,
3 y 40. El segundo calcula 8.33 y entra en la rama de aprobado. El
tercero imprime que el número es negativo mediante elif. Los originales
en C y sus traducciones terminan con código 0 y coinciden al ejecutarse.
La regresión contiene 33 pruebas de la etapa 3 y 68 de la etapa 4,
incluidas 37 comparaciones entre C y Python».

**Errores y límites.** «Un error léxico o sintáctico hace fallar el análisis.
En traducción también se comprueba la semántica. No se publica una salida
nueva si falla alguna fase. No es C completo: no admite ciclos, arreglos,
otras funciones ni todos los formatos de printf. La validación cubre el
subconjunto y los casos documentados, no todos los programas C».

**Referencia académica.** «Utilicé las calculadoras 01 y 02 para comprender
tokens, valores y variables, y los generadores 03, 04 y 05 para las
acciones, entrada desde archivo, tabla de símbolos y fprintf. El PDF
explica la comunicación Flex/Bison. La salida Python y las comprobaciones
adicionales son adaptaciones del proyecto. A diferencia del cálculo
anticipado del ejemplo 05, aquí conservo las expresiones».

## Preguntas para practicar

| Pregunta | Respuesta basada en la implementación |
|---|---|
| ¿Por qué no basta Flex? | Reconoce lexemas; Bison comprueba cómo se combinan en estructuras como declaraciones, bloques y expresiones. |
| ¿Qué cambia entre las dos modalidades? | Con un archivo solo se comprueban léxico y sintaxis. Con entrada y salida se activan además semántica y generación. |
| ¿Dónde se genera realmente Python? | Las acciones de parser.y llaman a traduccion.c y construyen fragmentos. publicar_programa los escribe mediante fprintf. |
| ¿Qué ocurre con $2? | Depende de la producción: en una suma es PLUS; en tipo ID es el identificador; en LPAREN expresion RPAREN es la expresión. |
| ¿Por qué bloque devuelve $3? | La acción intermedia que entra al ámbito cuenta como posición 2; lista_sentencias ocupa la posición 3. |
| ¿Por qué existen %destructor? | Para liberar valores que Bison descarte al abortar. Las reducciones normales liberan o transfieren sus valores por su cuenta. |
| ¿Cómo se resuelve 2 + 3 * 4? | termino forma primero 3 * 4 y expresion lo combina con 2. El resultado se calcula al ejecutar Python. |
| ¿Se usa %left? | No en este parser; la precedencia y asociatividad salen de los niveles y la recursión por la izquierda. |
| ¿Qué sucede con int x;? | Se registra sin inicializar y no se emite una asignación inventada. Una lectura alcanzable antes de asignar se rechaza. |
| ¿Se permite repetir un nombre? | En el mismo ámbito es error. En un ámbito interior puede sombrear otra variable y recibe nombre Python distinto si hace falta. |
| ¿Por qué se renombran algunas variables? | Para conservar ámbitos de C y evitar conflictos con palabras Python, print, int, float y auxiliares. |
| ¿La tabla guarda valores de las variables? | No; guarda metadatos. No se evalúan expresiones durante la traducción. |
| ¿Por qué no usar directamente //? | Con negativos redondea hacia menos infinito. _c_idiv reproduce el truncamiento entero hacia cero de C. |
| ¿Por qué el promedio tiene llamadas _c_f32? | C float almacena y opera en precisión de 32 bits en el perfil validado; Python float suele tener mayor precisión. El auxiliar conserva ese redondeo. |
| ¿T_DOUBLE amplía la gramática? | No. Es el tipo interno de literales reales sin sufijo, como 6.0. Solo se declaran int y float. |
| ¿Por qué print lleva end=""? | Para no añadir saltos distintos de los incluidos en la cadena original. |
| ¿Cómo se traduce else if? | La producción ELSE sentencia_if y generar_elif forman elif, con el bloque correspondiente. |
| ¿Qué pasa si return aparece en una rama? | Esa rama deja de continuar; al combinar estados no se exige inicialización posterior desde ella. |
| ¿Qué ocurre con errores y salidas anteriores? | Se devuelve 1 y no se publica nueva salida. El destino anterior se mantiene. |
| ¿Hay errores que solo aparecerán al ejecutar? | Sí, por ejemplo división por cero; no se hace una prueba general de seguridad de ejecución. |
| ¿Depende de los nombres ejemplo1.c, etc.? | No. Lee los archivos dados en argv; las pruebas usan nombres y contenidos adicionales. |
| ¿Funciona en cualquier sistema? | Se validó en Windows con MSYS2 UCRT64, incluso en una ruta con espacios. La publicación usa API Windows. |

Para estudiar las producciones completas use [GRAMATICA.md](GRAMATICA.md).
Para explicar las llamadas y la memoria use [ARQUITECTURA.md](ARQUITECTURA.md).
El registro del cierre está en [pruebas/ETAPA7.md](pruebas/ETAPA7.md).
