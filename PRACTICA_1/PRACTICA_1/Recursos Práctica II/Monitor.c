// Monitor.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "lib.h"

int main() {
    int shmid = shmget(SHM_KEY, NUM_ELEMENTOS * sizeof(Elemento), 0666 | IPC_CREAT);
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
    op.sem_flg = 0;

    for (int i = 0; i < NUM_ELEMENTOS; i++) {
        op.sem_op = -1;
        semop(semid, &op, 1);

        printf("ID: %d, Nombre: %s\n", elementos[i].id, elementos[i].nombre);

        op.sem_op = 1;
        semop(semid, &op, 1);
    }

    shmdt(elementos);

    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}
