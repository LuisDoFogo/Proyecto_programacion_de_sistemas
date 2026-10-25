#include <stdio.h>

int main() {
    float calificacion1 = 8.5;
    float calificacion2 = 9.0;
    float calificacion3 = 7.5;
    float promedio;
    promedio = (calificacion1 + calificacion2 + calificacion3) / 3;
    printf("El promedio es: %.2f\n", promedio);
    if (promedio >= 6.0) {
        printf("El alumno aprobo\n");
    } else {
        printf("El alumno no aprobo\n");
    }
    return 0;
}
