#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "lib.h"

#define MAX_FILENAME_LEN 100
#define MAX_LINE_LEN 70
#define MAX_INPUT_ROWS 1000
#define MAX_OUTPUT_ROWS 10000
#define MAX_CASA_APUESTA_LENGTH 20
#define MAX_CASAS_APUESTAS 10
#define MAX_ITEMS 10000
#define NUM_HILOS 3
#define MAX_LINE_LENGTH 256
#define MAX_USERS 1000

char PATH_FILES[20];
char* INVENTORY_FILE;
char* LOG_FILE;
int NUM_PROCESOS;
int SIMULATE_SLEEP;
int CASAS_APUESTAS;
int SIMULATE_SLEEP_MIN;
int SIMULATE_SLEEP_MAX;
int SIZE_FP;
int N;
int patron=0;
char* DIRECTORIO;
char* DIRECTORIOS_CASAS[MAX_CASAS_APUESTAS]= {NULL};


Elemento *shared_memory; // Puntero a la memoria compartida
PatronUsuario *shared_memory_patrones = NULL;
int shm_id_patrones;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t sem1;
sem_t sem2;
sem_t sem3;
sem_t sem1_p;
sem_t sem2_p;
sem_t sem3_p;

typedef struct Config
{
    char path_files[MAX_LINE_LENGTH];
    char inventory_file[MAX_LINE_LENGTH];
    char log_file[MAX_LINE_LENGTH];
    int num_procesos;
    int simulate_sleep;
} Config;

int get_config(const char* filename, Config* config)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        return -1; // Si no se puede abrir el archivo, retornamos un error
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fp))
    {
        // Buscamos cada variable en el archivo y la guardamos en la estructura correspondiente
        if (strncmp(line, "PATH_FILES=", 11) == 0)
        {
            strncpy(config->path_files, line + 11, MAX_LINE_LENGTH);
            config->path_files[strcspn(config->path_files, "\n")] = '\0';
        }
        else if (strncmp(line, "INVENTORY_FILE=", 15) == 0)
        {
            strncpy(config->inventory_file, line + 15, MAX_LINE_LENGTH);
            config->inventory_file[strcspn(config->inventory_file, "\n")] = '\0';
        }
        else if (strncmp(line, "LOG_FILE=", 9) == 0)
        {
            strncpy(config->log_file, line + 9, MAX_LINE_LENGTH);
            config->log_file[strcspn(config->log_file, "\n")] = '\0';
        }
        else if (strncmp(line, "NUM_PROCESOS=", 13) == 0)
        {
            config->num_procesos = atoi(line + 13);
        }
        else if (strncmp(line, "SIMULATE_SLEEP=", 15) == 0)
        {
            config->simulate_sleep = atoi(line + 15);
        }
    }

    fclose(fp);
    return 0;
}

void leerfichero()
{
    // Abre el archivo de configuraci�n
    FILE* conf_file = fopen("config.conf", "r");
    if (conf_file == NULL)
    {
        printf("No se pudo abrir el archivo de configuraci�n\n");
        exit(1);
    }

    // Lee y procesa cada l�nea de configuraci�n
    char buffer[MAX_LINE_LEN];
    char* name;
    char* value;
    while (fgets(buffer, MAX_LINE_LEN, conf_file) != NULL)
    {
        // Ignora las l�neas en blanco o que comiencen con #
        if (buffer[0] == '\n' || buffer[0] == '#')
        {
            continue;
        }

        // Busca el signo = para separar el nombre de la variable y su valor
        name = strtok(buffer, "=");
        value = strtok(NULL, "=");

        // Elimina los espacios en blanco alrededor del valor
        value[strcspn(value, "\n")] = 0;

        while (*value == ' ')
        {
            ++value;
        }
        char* end = value + strlen(value) - 1;
        while (end > value && *end == ' ')
        {
            --end;
        }
        *(end + 1) = 0;

        // Configura las variables correspondientes
        if (strcmp(name, "PATH_FILES") == 0)
        {
            // Configura la variable PATH_FILES con el valor le�do
            strcpy(PATH_FILES,value);
        }
        else if (strcmp(name, "INVENTORY_FILE") == 0)
        {
            // Configura la variable INVENTORY_FILE con el valor le�do
            INVENTORY_FILE=strdup(value);
        }
        else if (strcmp(name, "LOG_FILE") == 0)
        {
            // Configura la variable LOG_FILE con el valor le�do
            LOG_FILE = strdup(value);
        }
        else if(strcmp(name, "CASAS_APUESTAS")==0)
        {
            CASAS_APUESTAS=atoi(value);
        }
        else if (strcmp(name, "NUM_PROCESOS") == 0)
        {
            // Configura la variable NUM_PROCESOS con el valor le�do
            NUM_PROCESOS= atoi(value);

        }
        else if (strcmp(name, "SIMULATE_SLEEP_MIN") == 0)
        {
            // Configura la variable SIMULATE_SLEEP con el valor le�do
            SIMULATE_SLEEP_MIN = atoi(value);

        }
        else if (strcmp(name, "SIMULATE_SLEEP_MAX") == 0)
        {
            // Configura la variable SIMULATE_SLEEP con el valor le�do
            SIMULATE_SLEEP_MAX = atoi(value);

        }
        else if (strcmp (name, "DIRECTORIO") == 0)
        {
            DIRECTORIO = strdup(value);
            strcat(DIRECTORIO,"/");
        }
        else if (strcmp (name, "DIRECTORIOS_CASAS")==0)
        {
            char* field= strtok(value, ",");
            for(int i=0; i<CASAS_APUESTAS; i++)
            {
                DIRECTORIOS_CASAS[i]=malloc(MAX_FILENAME_LEN * sizeof(char));
                strcpy(DIRECTORIOS_CASAS[i],DIRECTORIO);
                strcat(DIRECTORIOS_CASAS[i],field);
                field=strtok(NULL,",");
            }
        }
        else if (strcmp(name, "SIZE_FP") == 0)
        {
            // Configura la variable SIZE_FP con el valor leido
            SIZE_FP = atoi(value);

        }
        else if(strcmp(name, "N")==0)
            N = atoi(value);
    }

    // Cierra el archivo de configuraci�n
    fclose(conf_file);

}

void dormir()
{
    int retardo = rand() % (SIMULATE_SLEEP_MAX - SIMULATE_SLEEP_MIN + 1) + SIMULATE_SLEEP_MIN;
    sleep(retardo);
}

typedef struct
{
    char id_usuario[MAX_LINE_LENGTH];
    int apuestas[MAX_USERS];
    int num_apuestas;
} User;

void* patron1(void* arg)
{
    int total_participaciones;
    int total_apuestas;
    int tiempo_actual;
    int tiempo_antiguo[MAX_USERS] = {0};
    char id_usuario[MAX_LINE_LENGTH];

    // Iterar sobre los elementos de la memoria compartida
    int i;
    for (i = 0; i < MAX_ITEMS; i++)
    {
        pthread_mutex_lock(&mutex);
        total_participaciones = shared_memory[i].TotalParticipaciones;
        strcpy(id_usuario, shared_memory[i].id_usuario);
        tiempo_actual = shared_memory[i].TotalApuestas;
        pthread_mutex_unlock(&mutex);

        if (total_participaciones > 5)
        {
            if (tiempo_actual - tiempo_antiguo[atoi(id_usuario)] < 5)
            {
                // Comprobar que el id_usuario comienza con "DNI"
                if (strncmp(id_usuario, "DNI", 3) == 0)
                {
                    patron = 1; // Establecer el número de patrón encontrado
                    // Escribir el número de patrón y el id_usuario en la segunda memoria compartida
                    pthread_mutex_lock(&mutex);
                    shared_memory_patrones[i].patron = patron;
                    strcpy(shared_memory_patrones[i].id_usuario, id_usuario);
                    pthread_mutex_unlock(&mutex);
                }
            }
            tiempo_antiguo[atoi(id_usuario)] = tiempo_actual;
        }

    }

    pthread_exit(NULL);
}

void* patron2(void* arg)
{
    //dormir();
    int tiempo_antiguo[MAX_USERS] = {0};
    int incremento = 10; // Tiempo en segundos para considerar incremento de apuestas
    char id_usuario[MAX_LINE_LENGTH];
    int total_apuestas;
    int tiempo_actual;


    // Iterar sobre los elementos de la memoria compartida
    int i;
    for (i = 0; i < MAX_ITEMS; i++)
    {
        pthread_mutex_lock(&mutex);
        strcpy(id_usuario, shared_memory[i].id_usuario);
        total_apuestas = shared_memory[i].TotalApuestas;
        tiempo_actual = shared_memory[i].TotalEstadoNeg;
        pthread_mutex_unlock(&mutex);

        if (tiempo_actual - tiempo_antiguo[atoi(id_usuario)] < incremento)
        {
            // Comprobar que el id_usuario comienza con "DNI"
            if (strncmp(id_usuario, "DNI", 3) == 0)
            {
                patron = 2;
            pthread_mutex_lock(&mutex);
            // Escribir el número de patrón y el id_usuario en la segunda memoria compartida
            shared_memory_patrones[i].patron = patron;
            strcpy(shared_memory_patrones[i].id_usuario, id_usuario);
            pthread_mutex_unlock(&mutex);
            }
        }
        tiempo_antiguo[atoi(id_usuario)] = tiempo_actual;



    }

    pthread_exit(NULL);
}

void* patron3(void* arg)
{
    //dormir();

    User users[MAX_USERS];
    int num_users = 0;

    int i;
    for (i = 0; i < MAX_USERS; i++)
    {
        strcpy(users[i].id_usuario, "");
        users[i].num_apuestas = 0;
    }

    // Iterar sobre los elementos de la memoria compartida
    for (i = 0; i < MAX_ITEMS; i++)
    {
        char id_usuario[MAX_LINE_LENGTH];
        pthread_mutex_lock(&mutex);
        strcpy(id_usuario, shared_memory[i].id_usuario);
        int total_apuestas = shared_memory[i].TotalApuestas;
        pthread_mutex_unlock(&mutex);

        // Buscar el id_usuario en el array de usuarios
        int user_index = -1;
        int j;
        for (j = 0; j < num_users; j++)
        {
            if (strcmp(users[j].id_usuario, id_usuario) == 0)
            {
                user_index = j;
                break;
            }
        }

        if (user_index == -1)
        {
            // Si no se ha encontrado el usuario, añadirlo al array
            strcpy(users[num_users].id_usuario, id_usuario);
            users[num_users].num_apuestas = 0;
            user_index = num_users;
            num_users++;
        }

        if (users[user_index].num_apuestas > 0 && strcmp(users[user_index].id_usuario, id_usuario) == 0)
        {
            // Si ya se han procesado apuestas para este usuario, comprobar si están en orden ascendente
            int *apuestas = users[user_index].apuestas;
            int num_apuestas = users[user_index].num_apuestas;
            int apuesta_anterior = apuestas[num_apuestas - 1];

            if (total_apuestas > apuesta_anterior)
            {
                // Comprobar que el id_usuario comienza con "DNI"
                if (strncmp(id_usuario, "DNI", 3) == 0)
                {
                   patron = 3;
                // Escribir el número de patrón y el id_usuario en la segunda memoria compartida
                pthread_mutex_lock(&mutex);
                shared_memory_patrones[i].patron = patron;
                strcpy(shared_memory_patrones[i].id_usuario, id_usuario);
                pthread_mutex_unlock(&mutex);
                }
            }
        }

        // Añadir la apuesta al array de apuestas del usuario
        if (users[user_index].num_apuestas < MAX_USERS)
        {
            users[user_index].apuestas[users[user_index].num_apuestas] = total_apuestas;
            users[user_index].num_apuestas++;
        }

    }

    pthread_exit(NULL);
}

int main()
{
    leerfichero();
    // Declarar el identificador de la segunda memoria compartida

    // Crear la segunda memoria compartida
    shm_id_patrones = shmget(SHM_KEY_2, MAX_ITEMS * sizeof(PatronUsuario), IPC_CREAT | 0666);
    if (shm_id_patrones == -1) {
        perror("Error al crear la segunda memoria compartida");
        exit(1);
    }

    // Adjuntar la segunda memoria compartida
    shared_memory_patrones = (PatronUsuario*)shmat(shm_id_patrones, NULL, 0);
    if (shared_memory_patrones == (PatronUsuario*)(-1)) {
        perror("Error al adjuntar la segunda memoria compartida");
        exit(1);
    }


    // Obtener el puntero a la memoria compartida existente
    int shmid = shmget(SHM_KEY, 0, 0);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }
    shared_memory = (Elemento*)shmat(shmid, NULL, 0);
    if (shared_memory == (Elemento*)-1)
    {
        perror("shmat");
        exit(1);
    }

    sem_init(&sem1, 0, 1); // Inicializar el semáforo del arreglo de patrones
    sem_init(&sem2, 0, 1);
    sem_init(&sem3, 0, 1);
    sem_init(&sem1_p, 0, 1);
    sem_init(&sem2_p, 0, 1);
    sem_init(&sem3_p, 0, 1);

    pthread_t thread1, thread2, thread3;



    pthread_create(&thread1, NULL, patron1, NULL);
    pthread_create(&thread2, NULL, patron2, NULL);
    pthread_create(&thread3, NULL, patron3, NULL); // Pasar shared_memory como argumento


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    dormir();

    exit(0);
}

