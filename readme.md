Los # en C se resuelven primero por un preprosesador, entonces cuando lo compilas primero se resuelven los # y despues el codigo

ejemplo el #include = "pega aca el contenido de este archivo"

#define SHM_NAME "/telemetria" = "en todo el codigo, donde diga SHM_NAME, reemplazalo por /telemetria

#define con parentesis: reemplazar con un "hueco"

#define LOCK(t) sem_wait(&(t)->mutex) = Cuando escriben LOCK(algo), el preprocesador lo reemplaza por sem_wait(&(t)->mutex)


#ifdef / #else / #endif: Es un if del preprocesador:

Quien define SIN_SYNC = Se define solo desde el comando de compilacion con la opcion -D:
    gcc -DSIN_SYNC sensor.c -o sensor_sin_sync

Que es sem_wait(&(t)-> mutex)
    t -> el puntero a la ficha compartida
    (t)->mutex -> el semaforo que esta dentro de esa ficha. Los parentesis alrededor de t son una precaucion tipica de las macros.
    &(t)->mutex -> La direccion de ese semaforo. Las funciones de semaforos necesitas saber donde esta ara poder modificarolo (igual que con sem_init).

    sem_wait -> "quiero la llave". Si esta libre, la toma y sigue. Si la tiene otro proceso, se queda epserando hasta que la devuelvan. 
    sem_post -> "devuelvo la llave". Si alguien estaba esperando. ahora puede pasar