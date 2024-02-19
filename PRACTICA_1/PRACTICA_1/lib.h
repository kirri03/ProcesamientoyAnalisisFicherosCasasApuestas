// lib.h
#ifndef COMPARTIDO_H
#define COMPARTIDO_H

#include <sys/types.h>

#define SEM_KEY 1234
#define SHM_KEY 9876

#define SEM_KEY_2 5678
#define SHM_KEY_2 2003
#define ID_LENGTH 7

typedef struct {
    char casa_apuesta[20];
    char fecha_inicio[20];
    char fecha_fin[20];
    char id_usuario[20];
    char id_juego[20];
    int TotalParticipaciones;
    float TotalApuestas;
    float TotalEstadoPos;
    float TotalEstadoNeg;
} Elemento;

typedef struct {
    int patron;
    char id_usuario[ID_LENGTH];
} PatronUsuario;

Elemento *elementos;

#endif // COMPARTIDO_H
