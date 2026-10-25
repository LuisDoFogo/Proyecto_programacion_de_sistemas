#include <stdio.h>
// comentario de linea
int main() {
    /* comentario de bloque */
    int x = 4;
    float y = 2.5;
    x = x + 3 * (2 - 1);
    if (x != 0) {
        printf("Valor: %d\n", x);
    } else {
        printf("Cero\n");
    }
    return 0;
}
