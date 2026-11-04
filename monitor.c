#include <stdio.h>      // printf, perror
#include <fcntl.h>      // O_RDWR
#include <sys/mman.h>   // shm_open, mmap, munmap
#include <unistd.h>     // close
#include "telemetria.h" // nuestro molde, SHM_NAME, LOCK y UNLOCK

#define LECTURAS 1000000        // cuantas veces lee el monitor
#define UMBRAL_PELIGRO 95.0     // a partir de que temperatura hay alerta

int main(void) {
    // Abrir la memoria compartida que YA creo el sensor (sin O_CREAT)
    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (fd == -1) { perror("shm_open (¿esta corriendo el sensor?)"); return 1; }

    // Mapearla: ahora t apunta a la misma memoria fisica que el t del sensor
    Telemetria *t = mmap(NULL, sizeof(Telemetria),
                         PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (t == MAP_FAILED) { perror("mmap"); return 1; }

    printf("Monitor iniciado. Memoria mapeada en la direccion virtual %p\n", (void *)t);
    printf("Leyendo %d veces...\n", LECTURAS);

    unsigned long inconsistentes = 0, alertas = 0;
    unsigned long ultimo_seq = 0, distintas = 0;              // para contar mediciones distintas
    unsigned long ultimo_corrupto = 0, mediciones_corruptas = 0; // para contar mediciones corruptas distintas

    for (int n = 0; n < LECTURAS; n++) {
        Telemetria c;           // ficha local, privada del monitor

        LOCK(t);                // tomo la llave
        c = *t;                 // copio toda la ficha compartida de una vez
        UNLOCK(t);              // devuelvo la llave enseguida
        // Si el seq cambio, es una medicion nueva que todavia no habiamos visto
        if(c.seq != ultimo_seq){
            distintas++;
            ultimo_seq = c.seq;
        }
        // Si no se cumple presion = 2*temp y vibracion = 3*temp, la ficha estaba a medio escribir
                if (c.presion != 2 * c.temperatura || c.vibracion != 3 * c.temperatura) {
            inconsistentes++;
            // Solo la mostramos si es una medicion corrupta que no habiamos visto
            if (c.seq != ultimo_corrupto) {
                mediciones_corruptas++;
                ultimo_corrupto = c.seq;
                if (mediciones_corruptas <= 5)
                    printf("  Lectura corrupta: seq=%lu temp=%.0f presion=%.0f vibracion=%.0f\n",
                           c.seq, c.temperatura, c.presion, c.vibracion);
            }
        } else if (c.temperatura > UMBRAL_PELIGRO) {
            alertas++; //dato valido y peligroso
        }
    }

    printf("\nResultado:\n");
    printf("  Lecturas totales:        %d\n", LECTURAS);
    printf(" Mediciones distintas:     %lu\n", distintas);
    printf("  Lecturas inconsistentes: %lu (%.4f%%)\n",
           inconsistentes, 100.0 * inconsistentes / LECTURAS);
    printf("  Mediciones corruptas:    %lu de %lu\n", mediciones_corruptas, distintas);
    printf("  Alertas de peligro:      %lu\n", alertas);

    // Limpieza: el monitor NO borra la memoria ni destruye el semaforo (eso es del sensor)
    munmap(t, sizeof(Telemetria));
    close(fd);
    return 0;
}