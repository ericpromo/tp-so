#ifndef TELEMETRIA_H
#define TELEMETRIA_H
#include <semaphore.h>
#define SHM_NAME "/telemetria"

typedef struct {
    sem_t mutex;
    unsigned log seq;
    double temperatura;
    double presion;
    double vibracion;
} Telemetria;

#ifdef SIN_SYNC
    #define LOCK(t)
    #define UNLOCL(t)
#else
    #define LOCK(t) sem_wait(&(t)->mutex)
    #define UNLOCK(t) sem_wait(&(t)->mutex)
#endif

#endif
