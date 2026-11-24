#include <stdio.h>

typedef struct {
    int edad;
    double promedio;
} Alumno;

int main(void) {
    // Punteros simples
    int edad = 20;
    int *p = &edad;

    printf("edad vale: %d\n", edad);
    printf("edad vive en: %p\n", (void *)&edad);
    printf("p guarda: %p\n", (void *)p);
    printf("lo que hay donde apunta p: %d\n", *p);

    *p = 50;
    printf("despues de *p = 50, edad vale: %d\n\n", edad);

    //Punteros a struct 
    Alumno a1;
    a1.edad = 20;
    a1.promedio = 8.5;

    Alumno *pa = &a1;
    pa->edad = 25;

    printf("a1.edad vale: %d\n", a1.edad);
    printf("pa->edad vale: %d\n", pa->edad);

    return 0;

}