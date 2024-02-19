#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    int numLineas = obtenerNumeroLineasCSV(nombreArchivo);

    int shmid = shmget(SHM_KEY, numLineas * sizeof(Elemento), 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    Elemento *elementos = (Elemento *)shmat(shmid, NULL, 0);
    if (elementos == (Elemento *)(-1)) {
        perror("shmat");
        exit(1);
    }

    int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) < 0) {
        perror("semop");
        exit(1);
    }

    FILE *archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char linea[MAX_LINE_LENGTH];
    int i = 0;

while (fgets(linea, sizeof(linea), archivo) && i < numLineas) {
    sscanf(linea, "%[^;];%[^;];%[^;];%[^;];%[^;];%d;%f;%f;%f",
           elementos[i].casa_apuesta,
           elementos[i].fecha_inicio,
           elementos[i].fecha_fin,
           elementos[i].id_usuario,
           elementos[i].id_juego,
           &elementos[i].TotalParticipaciones,
           &elementos[i].TotalApuestas,
           &elementos[i].TotalEstadoPos,
           &elementos[i].TotalEstadoNeg);
    i++;
}
fclose(archivo);
shmdt(elementos);

return 0;
}
