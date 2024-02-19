
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

    numLineas = 0;
    char linea[MAX_LINE_LENGTH];

    while (fgets(linea, sizeof(linea), archivo)) {
        numLineas++;
    }

    fclose(archivo);

    return numLineas;
}

void imprimirElementosMemoriaCompartida(Elemento *elementos, int numElementos) {
    for (int i = 0; i < numElementos; i++) {
        printf("Elemento %d:\n", i);
        printf("Casa de Apuesta: %s\n", elementos[i].casa_apuesta);
        printf("Fecha de Inicio: %s\n", elementos[i].fecha_inicio);
        printf("Fecha de Fin: %s\n", elementos[i].fecha_fin);
        printf("ID de Usuario: %s\n", elementos[i].id_usuario);
        printf("ID de Juego: %s\n", elementos[i].id_juego);
        printf("Total de Participaciones: %d\n", elementos[i].TotalParticipaciones);
        printf("Total de Apuestas: %.2f\n", elementos[i].TotalApuestas);
        printf("Total de Estado Positivo: %.2f\n", elementos[i].TotalEstadoPos);
        printf("Total de Estado Negativo: %.2f\n", elementos[i].TotalEstadoNeg);
        printf("-------------------------\n");
    }
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

    // Inicializar el semáforo
    if (sem_init(&semid, 1, 1) != 0) {
        perror("sem_init");
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

    sem_wait(&semid); // Bloquear el semáforo antes de acceder a la memoria compartida
    i++;
    sem_post(&semid); // Liberar el semáforo después de acceder a la memoria compartida
}

fclose(archivo);
shmdt(elementos);

// Destruir el semáforo
if (sem_destroy(&semid) != 0) {
    perror("sem_destroy");
    exit(1);
}

return 0;
}

