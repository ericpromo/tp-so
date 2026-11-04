#include <stdio.h>      // printf, perror
#include <fcntl.h>      // O_CREAT, O_RDWR (opciones para abrir)
#include <signal.h>     // signal (para manejar Ctrl+C)
#include <sys/mman.h>   // shm_open, mmap, munmap, shm_unlink
#include <unistd.h>     // ftruncate, usleep, close
#include "telemetria.h" // Archivo propio, va entre comillas porque significa "buscalo en mi carpeta", <> significa es una biblioteca del sistema.

// El sensor tiene un bucle que repite mientras corriendo valga 1. Para frenarlo se aprieta Ctrl+C.
// Normalmente eso mata el programa; con esto hacemos que termine el bucle y limpie antes de salir.
static volatile sig_atomic_t corriendo = 1;
static void cortar(int s) { (void)s; corriendo = 0; }

int main(void) {
    // Cuando llegue SIGINT (Ctrl+C), ejecuta cortar
    struct sigaction sa = {0};
    sa.sa_handler = cortar;
    sigaction(SIGINT, &sa, NULL);

    // Crear la memoria compartida
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    // Recien creada la memoria mide 0 bytes, ftruncate le da un tamaño.
    // sizeof(Telemetria) calcula cuantos bytes ocupa una ficha del molde Telemetria.
    if (ftruncate(fd, sizeof(Telemetria)) == -1) { perror("ftruncate"); return 1; }

    // Mapear la memoria
    Telemetria *t = mmap(NULL, sizeof(Telemetria),
                         PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (t == MAP_FAILED) { perror("mmap"); return 1; }
    /* mmap le dice al SO: "agrega esa memoria compartida a mi tabla de paginas y decime en que direccion quedo".
       Devuelve una direccion, y por eso la guardamos en un puntero a Telemetria.
       Lo que recibe:
        - NULL -> elegi vos en que direccion ponerla
        - sizeof(Telemetria) -> cuanto mapear
        - PROT_READ | PROT_WRITE -> voy a leer y escribir
        - MAP_SHARED -> los cambios los tienen que ver los otros procesos
        - fd -> cual memoria (el numero de ticket del paso 1)
        - 0 -> desde el principio */

    // Preparar el semaforo: compartido entre procesos (1), con la llave disponible (1)
    if (sem_init(&t->mutex, 1, 1) == -1) { perror("sem_init"); return 1; }

    printf("Sensor iniciado. Memoria mapeada en la direccion virtual %p\n", (void *)t);
    printf("Ctrl+C para terminar.\n");

    unsigned long i = 0;
    while (corriendo) {
        /* while(corriendo) -> repite mientras corriendo no sea 0. En C, 0 es falso y cualquier otro numero es verdadero.
           Cuando aprieten Ctrl+C, cortar lo pone en 0 y el bucle termina.
           i++ -> cuenta las mediciones.
           % es el resto de la division. Asi base va de 0 a 99 y vuelve a empezar, como una temperatura que sube y baja. */
        i++;
        double base = i % 100;

        LOCK(t);                    // tomo la llave
        t->seq = i;
        t->temperatura = base;
        usleep(5);                  // agranda la ventana de la condicion de carrera
        t->presion = base * 2.0;
        t->vibracion = base * 3.0;
        UNLOCK(t);                  // devuelvo la llave
        if (i % 10000 == 0) printf("Medicion %lu\n", i); // imprime una linea cada 10.000 mediciones

        usleep(100);                // pausa entre mediciones
    }

    // Limpieza al salir: lo que se abrio, se cierra
    printf("\nSensor terminado. Mediciones escritas: %lu\n", i);
    sem_destroy(&t->mutex);         // destruyo el semaforo
    munmap(t, sizeof(Telemetria));  // saco la memoria de mi tabla de paginas
    close(fd);                      // devuelvo el "ticket"
    shm_unlink(SHM_NAME);           // borro la memoria compartida del sistema
    return 0;
}