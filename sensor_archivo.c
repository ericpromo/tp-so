#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>   // flock (en lugar de sys/mman.h)
#include <unistd.h>
#include "telemetria.h"

static volatile sig_atomic_t corriendo = 1;
static void cortar(int s) { (void)s; corriendo = 0; }

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = cortar;
    sigaction(SIGINT, &sa, NULL);

    // ★ Abrir (o crear) el archivo, en lugar de shm_open + ftruncate + mmap
    int fd = open(ARCHIVO, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) { perror("open"); return 1; }

    printf("Sensor (archivo) iniciado. Escribiendo en %s\n", ARCHIVO);
    printf("Ctrl+C para terminar.\n");

    Medicion m;         // ★ ficha local: la armamos aca y despues la mandamos al archivo
    unsigned long i = 0;
    while (corriendo) {
        i++;
        double base = i % 100;

        // ★ Armamos la ficha en NUESTRA memoria (no hace falta llave, es privada)
        m.seq = i;
        m.temperatura = base;
        m.presion = base * 2.0;
        m.vibracion = base * 3.0;

        // ★ La escribimos en el archivo con la llave tomada
        flock(fd, LOCK_EX);
        if (pwrite(fd, &m, sizeof(m), 0) == -1) { perror("pwrite"); break; }
        flock(fd, LOCK_UN);

        if (i % 10000 == 0) printf("Medicion %lu\n", i);
        usleep(100);
    }

    printf("\nSensor terminado. Mediciones escritas: %lu\n", i);
    close(fd);          // ★ solo cerramos; el archivo queda en la carpeta
    return 0;
}