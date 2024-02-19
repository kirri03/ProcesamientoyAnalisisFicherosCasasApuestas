#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USUARIOS 1000 // Máximo número de usuarios que vamos a procesar

// Estructura que representa a un usuario
typedef struct {
    char id[10];
    int num_apuestas;
    int num_participaciones;
    char id_juego_anterior[10];
    char fecha_inicio_anterior[11];
} Usuario;

// Función para detectar el patrón 1
int detectar_patron_1(int num_apuestas_anterior, int num_apuestas_actual) {
    return num_apuestas_actual > 1.2 * num_apuestas_anterior;
}

// Función para detectar el patrón 2
int detectar_patron_2(int num_participaciones_anterior, int num_participaciones_actual) {
    return num_participaciones_actual > 1.2 * num_participaciones_anterior;
}

// Función para detectar el patrón 3
int detectar_patron_3(char id_juego_anterior[], char id_juego_actual[]) {
    return id_juego_anterior != id_juego_actual;
}

int main() {
    FILE *fp;
    char linea[100];
    char *campo;
    Usuario usuarios[MAX_USUARIOS];
    int num_usuarios = 0;

    fp = fopen("consolidated.csv", "r");
    if (fp == NULL) {
        printf("Error al abrir el archivo.\n");
        return 1;
    }

    // Leer el archivo línea por línea
    while (fgets(linea, sizeof(linea), fp) != NULL) {
        campo = strtok(linea, ";");
        int i = 0;
        char id_usuario[10];
        int total_apuestas=0;
        int total_participaciones=0;
        char id_juego[10];
        char fecha_inicio[11];

        // Recorrer los campos de la línea
        while (campo != NULL) {
            switch (i) {
                case 0: // Casa de apuestas (no nos interesa)
                    break;
                case 1: // Fecha inicio (la almacenamos para detectar el patrón 1)
                    strcpy(fecha_inicio, campo);
                    break;
                case 2: // Fecha fin (no nos interesa)
                    break;
                case 3: // Id usuario
                    strcpy(id_usuario, campo);
                    break;
                case 4:// Id juego (lo almacenamos para detectar el patrón 3)
                    strcpy(id_juego,campo);
                    break;
                case 6: // Total apuestas (lo almacenamos para detectar el patrón 2)
                    total_apuestas = atoi(campo);
                    break;
                case 5: // Total participaciones (no nos interesa)
                case 7: // Total estado positivo (no nos interesa)
                case 8: // Total estado negativo (no nos interesa)
                default:
                    break;
            }
            campo = strtok(NULL, ";");
            i++;
        }

        // Buscar al usuario en el arreglo de usuarios
        int indice_usuario = -1;
        for (int j = 0; j < num_usuarios; j++) {
            if (strcmp(usuarios[j].id, id_usuario) == 0) {
                indice_usuario = j;
                break;
            }
        }

        // Si el usuario no existe en el arreglo, agregarlo
        if (indice_usuario == -1) {
            strcpy(usuarios[num_usuarios].id,id_usuario);
            usuarios[num_usuarios].num_apuestas = 0;
            usuarios[num_usuarios].num_participaciones = 0;
            strcpy(usuarios[num_usuarios].id_juego_anterior,id_juego);
            strcpy(usuarios[num_usuarios].fecha_inicio_anterior, "0000-00-00");
            indice_usuario = num_usuarios;
            num_usuarios++;
        }

        // Detectar patrones de comportamiento ludópata
        if (detectar_patron_1(usuarios[indice_usuario].num_apuestas, total_apuestas)) {
            printf("El usuario %s presenta el patron 1 (aumento del numero de apuestas) en la fecha %s.\n", id_usuario, fecha_inicio);
        }

        if (detectar_patron_2(usuarios[indice_usuario].num_participaciones, total_participaciones)) {
            printf("El usuario %s presenta el patron 2 (aumento del numero de participaciones) en la fecha %s.\n", id_usuario, fecha_inicio);
        }

        if (detectar_patron_3(usuarios[indice_usuario].id_juego_anterior, id_juego)) {
            printf("El usuario %s presenta el patron 3 (participacion en varios juegos) en la fecha %s.\n", id_usuario, fecha_inicio);
        }

        // Actualizar la información del usuario en el arreglo
        usuarios[indice_usuario].num_apuestas = total_apuestas;
        usuarios[indice_usuario].num_participaciones = total_participaciones;
       strcpy(usuarios[indice_usuario].id_juego_anterior, id_juego);
        strcpy(usuarios[indice_usuario].fecha_inicio_anterior, fecha_inicio);
    }

    fclose(fp);
return 0;
}
