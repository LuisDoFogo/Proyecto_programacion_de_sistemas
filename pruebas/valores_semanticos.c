#include "traduccion.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static Expresion lexema(const char *texto, Tipo tipo)
{
    char *copia = copiar_lexema(texto, strlen(texto));
    assert(copia != NULL);
    return expresion_desde_lexema(copia, tipo);
}

int main(void)
{
    char original[] = "numero";
    char *copia = copiar_lexema(original, strlen(original));
    Expresion expresion;

    assert(copia != NULL);
    original[0] = 'X';
    assert(strcmp(copia, "numero") == 0);
    expresion = expresion_desde_lexema(copia, T_DESCONOCIDO);
    assert(expresion.tipo == T_DESCONOCIDO);
    liberar_expresion(&expresion);

    /* Se mantiene la agrupacion derecha de la resta. */
    expresion = crear_binaria(lexema("10", T_INT), "-",
        crear_binaria(lexema("4", T_INT), "-", lexema("2", T_INT), PREC_SUMA),
        PREC_SUMA);
    assert(expresion.texto != NULL);
    assert(strcmp(expresion.texto, "10 - (4 - 2)") == 0);
    assert(expresion.tipo == T_INT);
    liberar_expresion(&expresion);

    expresion = crear_binaria(
        crear_binaria(lexema("x", T_DESCONOCIDO), "+", lexema("1", T_INT), PREC_SUMA),
        "*", lexema("2", T_INT), PREC_PRODUCTO);
    assert(expresion.texto != NULL);
    assert(strcmp(expresion.texto, "(x + 1) * 2") == 0);
    assert(expresion.tipo == T_DESCONOCIDO);
    liberar_expresion(&expresion);

    expresion = crear_binaria(lexema("x", T_FLOAT), "+", lexema("2.5", T_DOUBLE), PREC_SUMA);
    assert(expresion.texto != NULL);
    assert(expresion.tipo == T_DOUBLE);
    liberar_expresion(&expresion);

    expresion = crear_binaria(lexema("x", T_FLOAT), "+", lexema("2", T_INT), PREC_SUMA);
    assert(expresion.texto != NULL);
    assert(expresion.tipo == T_FLOAT);
    liberar_expresion(&expresion);

    /* Una division por cero se conserva como texto, sin ejecutarla. */
    expresion = crear_binaria(lexema("1", T_INT), "/", lexema("0", T_INT), PREC_PRODUCTO);
    assert(expresion.texto != NULL);
    assert(strcmp(expresion.texto, "1 / 0") == 0);
    liberar_expresion(&expresion);

    expresion = crear_negacion(crear_negacion(lexema("3", T_INT)));
    assert(expresion.texto != NULL);
    assert(strcmp(expresion.texto, "-(-3)") == 0);
    liberar_expresion(&expresion);
    assert(expresion.texto == NULL);
    liberar_expresion(&expresion);

    puts("Valores semanticos: copia, tipos, agrupacion y liberacion correctos.");
    return 0;
}
