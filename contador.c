#include <stdio.h>
#include <unistd.h>

int main(void) {
    int contador = 0;

    while (contador < 10) {
        contador++;
        printf("PID %d -> contador = %d (vive en %p)\n", 
            getpid(), contador, (void *)&contador);
        sleep(1); //espera 1 segundo
    }

    return 0;
}
