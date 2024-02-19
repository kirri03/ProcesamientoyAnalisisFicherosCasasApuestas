#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "lib.h"

#define MAX_LINE_LENGTH 256

int obtenerNumeroLineasCSV(const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    int numLineas = 0;
    char linea[MAX_LINE_LENGTH];

    while (fgets(linea, sizeof(linea), archivo)) {
        numLineas++;
    }

    fclose(archivo);

    return numLineas;
}

int main() {
    const char *nombreArchivo = "consolidated.csv";
    int NUM_ELEMENTOS = obtenerNumeroLineasCSV(nombreArchivo);

    int shmid = shmget(SHM_KEY, 0, 0);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    Elemento *elementos = (Elemento *)shmat(shmid, NULL, 0);
    if (elementos == (Elemento *)(-1)) {
        perror("shmat");
        exit(1);
    }

    int semid = semget(SEM_KEY, 0, 0);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) < 0) {
        perror("semop");
        exit(1);
    }

    int i;
    for (i = 0; i < NUM_ELEMENTOS; i++) {
        printf("Elemento %d:\n", i + 1);
        printf("Casa de apuesta: %s\n", elementos[i].casa_apuesta);
        printf("Fecha de inicio: %s\n", elementos[i].fecha_inicio);
        printf("Fecha de fin: %s\n", elementos[i].fecha_fin);
        printf("ID de usuario: %s\n", elementos[i].id_usuario);
        printf("ID de juego: %s\n", elementos[i].id_juego);
        printf("Total de participaciones: %d\n", elementos[i].TotalParticipaciones);
        printf("Total de apuestas: %.2f\n", elementos[i].TotalApuestas);
        printf("Total de estado positivo: %.2f\n", elementos[i].TotalEstadoPos);
        printf("Total de estado negativo: %.2f\n", elementos[i].TotalEstadoNeg);
        printf("\n");
    }

    shmdt(elementos);

    return 0;
}
