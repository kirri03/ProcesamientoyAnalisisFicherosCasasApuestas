#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "lib.h"
#include <stdlib.h>

int main() {
    // Obtener el ID de la memoria compartida
    int shmid = shmget(SHM_KEY, 0, 0);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Adjuntar la memoria compartida al espacio de direcciones del proceso
    Elemento *elementos = (Elemento *)shmat(shmid, NULL, 0);
    if (elementos == (Elemento *)(-1)) {
        perror("shmat");
        exit(1);
    }

    // Acceder a los elementos guardados en la memoria compartida
    for (int i = 0; i < numLineas; i++) {
        // Realizar operaciones en los elementos
        // Por ejemplo, imprimir los valores de un elemento en la consola
        printf("Elemento %d: %s\n", i, elementos[i].casa_apuesta);
    }

    // Desvincular la memoria compartida del espacio de direcciones del proceso
    shmdt(elementos);

    return 0;
}
