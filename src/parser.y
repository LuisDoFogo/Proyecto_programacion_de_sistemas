%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
int yylex_destroy(void);
void yyerror(const char *mensaje);
extern int errores_lexicos;

static int errores_sintacticos = 0;
static char *cuerpo_programa = NULL;
%}

%code requires {
#include "traduccion.h"
}

%define parse.error verbose
%locations

%union {
    char *texto;
    const char *operador;
    Tipo tipo;
    Expresion expresion;
    Argumentos argumentos;
    int simbolo;
    ContextoIf *contexto;
}

%token INCLUDE HEADER
%token INT FLOAT MAIN IF ELSE RETURN PRINTF
%token <texto> ID INT_LITERAL FLOAT_LITERAL STRING
%token GE LE EQ NE GT LT
%token ASSIGN PLUS MINUS MULT DIV
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA
%token ERROR_LEXICO

%type <tipo> tipo
%type <operador> operador_relacional
%type <expresion> expresion termino factor condicion
%type <argumentos> lista_argumentos
%type <simbolo> cabecera_declaracion
%type <contexto> inicio_if
%type <texto> definicion_funcion bloque lista_sentencias sentencia
%type <texto> declaracion asignacion llamada_printf sentencia_if parte_else sentencia_return

/* Bison libera los valores pendientes cuando interrumpe el analisis. */
%destructor { free($$); } <texto>
%destructor { liberar_expresion(&$$); } <expresion>
%destructor { liberar_argumentos(&$$); } <argumentos>
%destructor { liberar_contexto_if($$); } <contexto>

%start programa

%%

programa
    : directiva_inclusion definicion_funcion
        { cuerpo_programa = $2; }
    ;

directiva_inclusion
    : INCLUDE HEADER
    ;

definicion_funcion
    : INT MAIN LPAREN RPAREN bloque
        { $$ = $5; }
    ;

bloque
    : LBRACE { entrar_ambito(); } lista_sentencias RBRACE
        { salir_ambito(); $$ = $3; }
    ;

lista_sentencias
    : lista_sentencias sentencia
        { $$ = unir_textos($1, $2); }
    | sentencia
        { $$ = $1; }
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
        { $$ = texto_vacio(); }
    | cabecera_declaracion ASSIGN expresion SEMICOLON
        { $$ = generar_asignacion($1, $3); }
    ;

/* El nombre entra en su ambito antes de analizar el inicializador. */
cabecera_declaracion
    : tipo ID
        {
            $$ = declarar_simbolo($2, $1, @2.first_line, @2.first_column);
            free($2);
        }
    ;

tipo
    : INT   { $$ = T_INT; }
    | FLOAT { $$ = T_FLOAT; }
    ;

asignacion
    : ID ASSIGN expresion SEMICOLON
        {
            int indice = buscar_simbolo($1, @1.first_line, @1.first_column);
            $$ = generar_asignacion(indice, $3);
            free($1);
        }
    ;

llamada_printf
    : PRINTF LPAREN STRING RPAREN SEMICOLON
        {
            Argumentos argumentos = {NULL, 0};
            $$ = generar_printf($3, argumentos, @3.first_line, @3.first_column);
        }
    | PRINTF LPAREN STRING COMMA lista_argumentos RPAREN SEMICOLON
        { $$ = generar_printf($3, $5, @3.first_line, @3.first_column); }
    ;

lista_argumentos
    : lista_argumentos COMMA expresion
        { $$ = agregar_argumento($1, $3); }
    | expresion
        { $$ = primer_argumento($1); }
    ;

sentencia_if
    : inicio_if bloque { preparar_else($1); } parte_else
        { $$ = terminar_if($1, $2, $4); }
    ;

inicio_if
    : IF LPAREN condicion RPAREN
        { $$ = iniciar_if($3); }
    ;

parte_else
    : %empty
        { $$ = texto_vacio(); }
    | ELSE bloque
        { $$ = generar_else($2); }
    | ELSE sentencia_if
        { $$ = generar_elif($2); }
    ;

condicion
    : expresion operador_relacional expresion
        {
            $$ = crear_comparacion($1, $2, $3);
            if ($$.texto == NULL) { YYNOMEM; }
        }
    ;

operador_relacional
    : GE { $$ = ">="; }
    | LE { $$ = "<="; }
    | EQ { $$ = "=="; }
    | NE { $$ = "!="; }
    | GT { $$ = ">"; }
    | LT { $$ = "<"; }
    ;

sentencia_return
    : RETURN expresion SEMICOLON
        { $$ = generar_return($2); }
    ;

expresion
    : expresion PLUS termino
        {
            $$ = crear_binaria($1, "+", $3, PREC_SUMA);
            if ($$.texto == NULL) { YYNOMEM; }
        }
    | expresion MINUS termino
        {
            $$ = crear_binaria($1, "-", $3, PREC_SUMA);
            if ($$.texto == NULL) { YYNOMEM; }
        }
    | termino
        { $$ = $1; }
    ;

termino
    : termino MULT factor
        {
            $$ = crear_binaria($1, "*", $3, PREC_PRODUCTO);
            if ($$.texto == NULL) { YYNOMEM; }
        }
    | termino DIV factor
        {
            $$ = crear_binaria($1, "/", $3, PREC_PRODUCTO);
            if ($$.texto == NULL) { YYNOMEM; }
        }
    | factor
        { $$ = $1; }
    ;

factor
    : ID
        { $$ = referencia_variable($1, @1.first_line, @1.first_column); }
    | INT_LITERAL
        { $$ = literal_numerico($1, T_INT, @1.first_line, @1.first_column); }
    | FLOAT_LITERAL
        { $$ = literal_numerico($1, T_DOUBLE, @1.first_line, @1.first_column); }
    | LPAREN expresion RPAREN
        { $$ = $2; }
    | MINUS factor
        {
            $$ = crear_negacion($2);
            if ($$.texto == NULL) { YYNOMEM; }
        }
    ;

%%

void yyerror(const char *mensaje)
{
    ++errores_sintacticos;
    fprintf(stderr,
            "Error sintactico en la linea %d, columna %d: %s\n",
            yylloc.first_line,
            yylloc.first_column,
            mensaje);
}

int main(int argc, char **argv)
{
    extern FILE *yyin;
    int resultado;
    int correcto;

    if (argc > 3) {
        fprintf(stderr, "Uso: %s [archivo.c [salida.py]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc >= 2) {
        yyin = fopen(argv[1], "r");

        if (yyin == NULL) {
            perror("No se pudo abrir el archivo");
            return EXIT_FAILURE;
        }
    }

    iniciar_traduccion(argc == 3);
    resultado = yyparse();

    if (argc >= 2 && yyin != NULL) {
        fclose(yyin);
    }
    yylex_destroy();

    correcto = resultado == 0 && errores_sintacticos == 0 &&
               errores_lexicos == 0 && errores_semanticos == 0;
    if (correcto && argc == 3) {
        correcto = publicar_programa(argv[1], argv[2], cuerpo_programa);
    }
    if (correcto) {
        if (argc == 3) {
            printf("Traduccion correcta: codigo Python guardado en %s\n", argv[2]);
        } else {
            puts("Analisis sintactico correcto: el archivo cumple la gramatica.");
        }
    } else {
        puts(argc == 3 ? "Traduccion fallida: no se publico una nueva salida."
                       : "Analisis sintactico fallido.");
    }
    free(cuerpo_programa);
    finalizar_traduccion();
    return correcto ? EXIT_SUCCESS : EXIT_FAILURE;
}
