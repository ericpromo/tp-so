#include <stdio.h>
#include <fcntl.h>      // open, O_RDONLY
#include <sys/file.h>   // flock
#include <time.h>       // clock_gettime (para medir el tiempo)
#include <unistd.h>     // pread, close
#include "telemetria.h"

#define LECTURAS 1000000

int main(void) {
    // Abrir el archivo que creo el sensor, solo para leer
    int fd = open(ARCHIVO, O_RDONLY);
    if (fd == -1) { perror("open (¿esta corriendo el sensor_archivo?)"); return 1; }

    printf("Monitor (archivo) iniciado. Leyendo %d veces de %s...\n", LECTURAS, ARCHIVO);

    unsigned long inconsistentes = 0, ultimo_seq = 0, distintas = 0;

    // Anoto la hora de inicio
    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int n = 0; n < LECTURAS; n++) {
        Medicion c;

        flock(fd, LOCK_SH);                              // tomo la llave (compartida)
        ssize_t leidos = pread(fd, &c, sizeof(c), 0);    // leo la ficha desde el principio del archivo
        flock(fd, LOCK_UN);                              // devuelvo la llave

        if (leidos != sizeof(c)) continue;   // si no se leyo la ficha completa, salteo esta vuelta

        if (c.seq != ultimo_seq) {
            distintas++;
            ultimo_seq = c.seq;
        }
        if (c.presion != 2 * c.temperatura || c.vibracion != 3 * c.temperatura)
            inconsistentes++;
    }

    // Anoto la hora de fin y calculo cuanto tardo
    clock_gettime(CLOCK_MONOTONIC, &fin);
    double segundos = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("\nResultado:\n");
    printf("  Lecturas totales:        %d\n", LECTURAS);
    printf("  Mediciones distintas:    %lu\n", distintas);
    printf("  Lecturas inconsistentes: %lu\n", inconsistentes);
    printf("  Tiempo total:            %.3f segundos\n", segundos);
    printf("  Tiempo por lectura:      %.1f nanosegundos\n", segundos * 1e9 / LECTURAS);
    
    close(fd);
    return 0;
}