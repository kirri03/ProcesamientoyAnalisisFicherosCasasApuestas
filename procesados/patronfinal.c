#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_LINE_LENGTH 1024
#define MAX_USERS 100


sem_t sem1;
sem_t sem2;
sem_t sem3;
sem_t varglobal;

typedef struct {
    char id_usuario[MAX_LINE_LENGTH];
    int apuestas[MAX_USERS];
    int num_apuestas;
} User;


void* patron1(void* arg) {
    int total_participaciones;
    int total_apuestas;
    int tiempo_actual;
    int tiempo_antiguo[MAX_USERS] = {0};
    char id_usuario[MAX_LINE_LENGTH];

    FILE* fp = fopen("consolidated.csv", "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo");
        pthread_exit(NULL);
    }

    char buffer[MAX_LINE_LENGTH];
    fgets(buffer, MAX_LINE_LENGTH, fp); // leer primera línea

    while (fgets(buffer, MAX_LINE_LENGTH, fp)) {
        char* token = strtok(buffer, ";");
        char* tokens[9];
        int i = 0;

        while (token) {
            tokens[i++] = token;
            token = strtok(NULL, ";");
        }

        total_participaciones = atoi(tokens[5]);
        strcpy(id_usuario, tokens[3]);
        tiempo_actual = atoi(tokens[7]);

        sem_wait(&sem1);
        if (total_participaciones > 5) {
            if (tiempo_actual - tiempo_antiguo[atoi(id_usuario)] < 5) {

                printf("Se ha encontrado el patron 1 para el usuario con DNI: %s\n", id_usuario);

            }
            tiempo_antiguo[atoi(id_usuario)] = tiempo_actual;

        }
        sem_post(&sem2);
    }

 fclose(fp);
    pthread_exit(NULL);
}

void* patron2(void* arg)
{
// Abrir archivo CSV
    FILE *fp = fopen("consolidated.csv", "r");
    if (fp == NULL) {
        printf("No se pudo abrir el archivo\n");
        pthread_exit(NULL);
    }

    // Leer primera línea
    char line[MAX_LINE_LENGTH];
    fgets(line, MAX_LINE_LENGTH, fp);
    char *idUsuario = strtok(line, ";"); // Obtener valor del IdUsuario

    char *idJuego;

    // Recorrer archivo para cada IdUsuario
    while (idUsuario != NULL) {
        // Almacenar IdJuegos para el IdUsuario actual
        int arraySize = 0;
        int *idJuegos = NULL;
        while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {

            char *token = strtok(line, ";"); // saltar casa apuesta
            token = strtok(NULL, ";"); // Saltar fecha_inicio
            token = strtok(NULL, ";"); // Saltar fecha_fin
            token = strtok(NULL, ";");
            strcpy(idUsuario, token);
            if (strcmp(token, idUsuario) == 0) {
                token = strtok(NULL, ";"); // Saltar casaapuesta
                token = strtok(NULL, ";"); // Saltar fecha_inicio
                token = strtok(NULL, ";"); // Saltar fecha_fin
                token = strtok(NULL, ";"); // Saltar idUsuario
                idJuego = strtok(NULL, ";");
                arraySize++;
                token = strtok(NULL, ";"); // Saltar TotalParticipaciones
                token = strtok(NULL, ";"); // Saltar TotalApuestas
                token = strtok(NULL, ";"); // Saltar TotalEstadoPos
                token = strtok(NULL, ";"); // Saltar TotalEstadosNeg
            }
        }

        // Verificar si hay más de un IdJuego
        sem_wait(&sem2);
        if (arraySize > 1) {

            printf("Se ha encontrado el patron 2 para el usuario con DNI: %s\n", idUsuario);

        }
        sem_post(&sem3);
        // Vaciar array dinámico
        free(idJuegos);

        // Leer siguiente IdUsuario
        fgets(line, MAX_LINE_LENGTH, fp);
        idUsuario = strtok(line, ";");
    }
     fclose(fp);
    pthread_exit(NULL);
}

void* patron3(void* arg)
{
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    User users[MAX_USERS];
    int num_users = 0;

    fp = fopen("consolidated.csv", "r");
    if (fp == NULL) {
        printf("Error al abrir el archivo\n");
        pthread_exit(NULL);
    }

    // Leer la primera línea del archivo y guardar el id_usuario
    if (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        char *token = strtok(line, ";");
        token = strtok(NULL, ";"); // Saltar CASA APUESTA
        token = strtok(NULL, ";"); // Saltar FECHA_INICIO
        token = strtok(NULL, ";"); // Saltar FECHA_FIN
        strcpy(users[num_users].id_usuario, token);
        users[num_users].num_apuestas = 0;
        num_users++;
    }
    // Procesar el resto del archivo
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        char *token = strtok(line, ";");
        token = strtok(NULL, ";"); // Saltar casa apuesta
        token = strtok(NULL, ";"); // Saltar FECHA_INICIO
        token = strtok(NULL, ";"); // Saltar FECHA_FIN

        char id_usuario[MAX_LINE_LENGTH];
        strcpy(id_usuario, token);
        token = strtok(NULL, ";"); // Saltar IdUsuario
        token = strtok(NULL, ";"); // Saltar IdJuego
        token = strtok(NULL, ";"); // Saltar TotalParticipaciones
        int total_apuestas = atoi(strtok(NULL, ";"));
        token = strtok(NULL, ";"); // Saltar TotalEstadoPos
        token = strtok(NULL, ";"); // Saltar TotalEstadosNeg

        // Buscar el id_usuario en el array de usuarios
        int user_index = -1;
        sem_wait(&varglobal);
        for (int i = 0; i < num_users; i++) {
            if (strcmp(users[i].id_usuario, id_usuario) == 0) {
                user_index = i;
                break;
            }
        }

        if (user_index == -1) {
            // Si no se ha encontrado el usuario, añadirlo al array
            strcpy(users[num_users].id_usuario, id_usuario);
            users[num_users].num_apuestas = 0;
            user_index = num_users;
            num_users++;
        }

        if (users[user_index].num_apuestas > 0 && strcmp(users[user_index].id_usuario, id_usuario) == 0) {
            // Si ya se han procesado apuestas para este usuario, comprobar si están en orden ascendente
            int *apuestas = users[user_index].apuestas;
            int num_apuestas = users[user_index].num_apuestas;
            int apuesta_anterior = apuestas[num_apuestas - 1];
            sem_wait(&sem3);
            if (total_apuestas > apuesta_anterior) {

                printf("Se ha encontrado el patron 3 para el usuario con DNI: %s\n", id_usuario);

            }

        }
            sem_post(&sem1);
        // Añadir la apuesta al array de apuestas del usuario
        if (users[user_index].num_apuestas < MAX_USERS) {
            users[user_index].apuestas[users[user_index].num_apuestas] = total_apuestas;
            users[user_index].num_apuestas++;
        }
        sem_post(&varglobal);
    }
     fclose(fp);
    pthread_exit(NULL);
}

int main()
{
    FILE* file = fopen("consolidated.csv", "r");
    if (!file)
    {
        printf("Error: no se puede abrir el archivo.\n");
        exit(1);
    }

    sem_init(&sem1, 0, 1); // Inicializar el semáforo del arreglo de patrones
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 0);

    sem_init(&varglobal, 0, 1);


    pthread_t thread1, thread2, thread3;

    pthread_create(&thread1, NULL, patron1, (void*) file);
    pthread_create(&thread2, NULL, patron2, (void*) file);
    pthread_create(&thread3, NULL, patron3, (void*) file);


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);



    fclose(file);
    return 0;
}


