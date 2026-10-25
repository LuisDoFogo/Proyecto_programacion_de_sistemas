%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *mensaje);
extern int yylineno;

static int errores_sintacticos = 0;
%}

%define parse.error verbose

%token INCLUDE HEADER
%token INT FLOAT MAIN IF ELSE RETURN PRINTF
%token ID INT_LITERAL FLOAT_LITERAL STRING
%token GE LE EQ NE GT LT
%token ASSIGN PLUS MINUS MULT DIV
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA

%start programa

%%

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
    : LBRACE lista_sentencias RBRACE
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
    : tipo ID SEMICOLON
    | tipo ID ASSIGN expresion SEMICOLON
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
    : IF LPAREN condicion RPAREN bloque parte_else
    ;

parte_else
    : /* vacio */
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

%%

void yyerror(const char *mensaje)
{
    ++errores_sintacticos;
    fprintf(stderr,
            "Error sintactico en la linea %d: %s\n",
            yylineno,
            mensaje);
}

int main(int argc, char **argv)
{
    extern FILE *yyin;
    int resultado;

    if (argc > 2) {
        fprintf(stderr, "Uso: %s [archivo.c]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        yyin = fopen(argv[1], "r");

        if (yyin == NULL) {
            perror("No se pudo abrir el archivo");
            return EXIT_FAILURE;
        }
    }

    resultado = yyparse();

    if (argc == 2 && yyin != NULL) {
        fclose(yyin);
    }

    if (resultado == 0 && errores_sintacticos == 0) {
        puts("Analisis sintactico correcto: el archivo cumple la gramatica.");
        return EXIT_SUCCESS;
    }

    puts("Analisis sintactico fallido.");
    return EXIT_FAILURE;
}
