%option noyywrap yylineno noinput nounput nodefault
%x COMENTARIO
%{
#include <stdio.h>
#include "parser.tab.h"

int errores_lexicos = 0;

static int siguiente_linea = 1;
static int siguiente_columna = 1;
static YYLTYPE inicio_comentario;

/* Se actualiza tambien al descartar espacios y comentarios. */
static void actualizar_ubicacion(const char *texto, size_t longitud)
{
    size_t i;
    yylloc.first_line = siguiente_linea;
    yylloc.first_column = siguiente_columna;
    yylloc.last_line = siguiente_linea;
    yylloc.last_column = siguiente_columna;

    for (i = 0; i < longitud; ++i) {
        yylloc.last_line = siguiente_linea;
        yylloc.last_column = siguiente_columna;
        if (texto[i] == '\n') {
            ++siguiente_linea;
            siguiente_columna = 1;
        } else {
            ++siguiente_columna;
        }
    }
}

static int error_lexico(const char *mensaje)
{
    ++errores_lexicos;
    fprintf(stderr, "Error lexico en la linea %d, columna %d: %s\n",
            yylloc.first_line, yylloc.first_column, mensaje);
    return ERROR_LEXICO;
}

static int token_con_lexema(int token, const char *texto, size_t longitud)
{
    yylval.texto = copiar_lexema(texto, longitud);
    if (yylval.texto == NULL) {
        return error_lexico("memoria insuficiente para copiar el lexema");
    }
    return token;
}

#define YY_USER_ACTION actualizar_ubicacion(yytext, (size_t)yyleng);
%}
DIGITO      [0-9]
FLOTANTE    ({DIGITO}+"."{DIGITO}*|{DIGITO}*"."{DIGITO}+)
IDENT       [a-zA-Z_][a-zA-Z0-9_]*
CADENA      \"([^\"\\\r\n\x00]|\\[^\r\n\x00])*\"
%%
[ \t\r\n]+                  ;
"//"[^\n\x00]*              ;
"/*"                        { inicio_comentario = yylloc; BEGIN(COMENTARIO); }
<COMENTARIO>"*/"            { BEGIN(INITIAL); }
<COMENTARIO>[^*\n\x00]+     ;
<COMENTARIO>"*"             ;
<COMENTARIO>\n              ;
<COMENTARIO><<EOF>>         {
                                yylloc = inicio_comentario;
                                BEGIN(INITIAL);
                                return error_lexico("comentario sin cerrar");
                            }
"#include"                  { return INCLUDE; }
"<stdio.h>"                 { return HEADER; }
"int"                       { return INT; }
"float"                     { return FLOAT; }
"main"                      { return MAIN; }
"if"                        { return IF; }
"else"                      { return ELSE; }
"return"                    { return RETURN; }
"printf"                    { return PRINTF; }
">="                        { return GE; }
"<="                        { return LE; }
"=="                        { return EQ; }
"!="                        { return NE; }
">"                         { return GT; }
"<"                         { return LT; }
"="                         { return ASSIGN; }
"+"                         { return PLUS; }
"-"                         { return MINUS; }
"*"                         { return MULT; }
"/"                         { return DIV; }
"("                         { return LPAREN; }
")"                         { return RPAREN; }
"{"                         { return LBRACE; }
"}"                         { return RBRACE; }
";"                         { return SEMICOLON; }
","                         { return COMMA; }
{FLOTANTE}                  { return token_con_lexema(FLOAT_LITERAL, yytext, (size_t)yyleng); }
{DIGITO}+                   { return token_con_lexema(INT_LITERAL, yytext, (size_t)yyleng); }
{CADENA}                    { return token_con_lexema(STRING, yytext, (size_t)yyleng); }
{IDENT}                     { return token_con_lexema(ID, yytext, (size_t)yyleng); }
\"                         { return error_lexico("cadena sin cerrar o con contenido invalido"); }
<*>\x00                    { return error_lexico("byte NUL no permitido"); }
.                           {
                                char mensaje[80];
                                snprintf(mensaje, sizeof mensaje,
                                         "caracter no reconocido '%s' (0x%02X)",
                                         yytext, (unsigned int)(unsigned char)yytext[0]);
                                return error_lexico(mensaje);
                            }
<<EOF>>                     {
                                actualizar_ubicacion("", 0);
                                return 0;
                            }
%%
