#ifndef TELEMETRIA_H
#define TELEMETRIA_H
#include <semaphore.h>
#define SHM_NAME "/telemetria"

typedef struct {
    sem_t mutex;
    unsigned long seq;
    double temperatura;
    double presion;
    double vibracion;
} Telemetria;

#define ARCHIVO "telemetria.dat"

typedef struct {
    unsigned long seq;
    double temperatura;
    double presion;
    double vibracion;
} Medicion;

#ifdef SIN_SYNC
    #define LOCK(t)
    #define UNLOCK(t)
#else
    #define LOCK(t) sem_wait(&(t)->mutex)
    #define UNLOCK(t) sem_post(&(t)->mutex)
#endif

#endif
