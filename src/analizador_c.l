%option noyywrap yylineno
%{
#include <stdio.h>
#include "parser.tab.h"
%}
DIGITO      [0-9]
FLOTANTE    ({DIGITO}+"."{DIGITO}*|{DIGITO}*"."{DIGITO}+)
IDENT       [a-zA-Z_][a-zA-Z0-9_]*
CADENA      \"([^\\\n]|\\.)*\"
%%
[ \t\r\n]+                  ;
"//"[^\n]*                  ;
"/*"([^*]|\*+[^*/])*\*+"/"  ;
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
{FLOTANTE}                  { return FLOAT_LITERAL; }
{DIGITO}+                   { return INT_LITERAL; }
{CADENA}                    { return STRING; }
{IDENT}                     { return ID; }
. { fprintf(stderr, "Error lexico en la linea %d: caracter no reconocido '%s'\n", yylineno, yytext); return yytext[0]; }
%%
