#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
cd src
echo "[1/3] Generando el parser con Bison..."
bison -d -Wall -Werror=conflicts-sr -Werror=conflicts-rr -o parser.tab.c parser.y
echo "[2/3] Generando el lexer con Flex..."
flex -o lex.yy.c analizador_c.l
echo "[3/3] Compilando..."
gcc -Wall -Wextra -o ../compilador.exe parser.tab.c lex.yy.c traduccion.c
echo "Listo: compilador.exe"
