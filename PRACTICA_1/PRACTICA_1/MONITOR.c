#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LINE_LEN 70
#define MAX_USERS 1000

char PATH_FILES[20];
char* INVENTORY_FILE;
char* LOG_FILE;
int NUM_PROCESOS;
int SIMULATE_SLEEP;
int CASAS_APUESTAS;
int SIMULATE_SLEEP_MIN;
int SIMULATE_SLEEP_MAX;

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
    }

    // Cierra el archivo de configuraci�n
    fclose(conf_file);

}

void dormir() {
    int retardo = rand() % (SIMULATE_SLEEP_MAX - SIMULATE_SLEEP_MIN + 1) + SIMULATE_SLEEP_MIN;
    sleep(retardo);
}

void registrarInstancia(char* INVENTORY_FILE) {
    FILE *fp;
    int lineas = 0;
    int no_apuestas_consolidadas = 0;
    int c = 0;
    int a = 0;

    time_t rawtime;
    struct tm *timeinfo;

    // Obtener la fecha y hora actual
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Abrir el archivo consolidado en modo de lectura
    fp = fopen(INVENTORY_FILE, "r");

    // Contar el número de líneas en el archivo
    if (fp) {
        while ((c = getc(fp)) != EOF) {
            if (c == '\n') {
                no_apuestas_consolidadas++;
            }
        }
        fclose(fp);
    } else {
        printf("No se pudo abrir el archivo %s.\n", INVENTORY_FILE);
    }

    // Verificar si el archivo de registro existe
    fp = fopen("LOG.log", "r");



    if (fp == NULL)
    {
        // Si no existe, crearlo en modo escritura y escribir el encabezado y los datos de la primera instancia
        fp = fopen("LOG.log", "w");
        if (fp == NULL)
        {
            printf("No se pudo crear el archivo de registro.\n");
        }

        fprintf(fp, "FECHA:::HORA:::NoPROCESO:::INICIO:::FIN:::NOMBRE_FICHERO:::NoApuestasConsolidadas\n");
        fprintf(fp, "%d-%02d-%02d:::%02d:%02d:%02d.%02d:::%d:::%02d:%02d:%02d.%02d:::%02d:%02d:%02d.%02d:::%s:::%d\n",
                timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_isdst,
                1, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_isdst,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_isdst,
                INVENTORY_FILE, no_apuestas_consolidadas);
        printf("Se ha creado el archivo de registro.\n");

    }
    else
    {
        // Abrir el archivo de registro en modo de lectura para contar el número de líneas
        fp = fopen("LOG.log", "r");
        lineas = 0;
        while ((a = getc(fp)) != EOF) {
            if (a == '\n') {
                lineas++;
            }
        }
        fclose(fp);

        // Abrir el archivo de registro en modo de escritura, pero sin sobreescribir los datos anteriores
        fp = fopen("LOG.log", "a");

        // Escribir los datos de la nueva instancia en una nueva línea
        fprintf(fp, "%d-%02d-%02d:::%02d:%02d:%02d.%02d:::%d:::%02d:%02d:%02d.%02d:::%02d:%02d:%02d.%02d:::%s:::%d\n",
                timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_isdst,
                lineas, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,timeinfo->tm_isdst,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,timeinfo->tm_isdst,
                INVENTORY_FILE, no_apuestas_consolidadas);
            // Contar el número de líneas en el archivo


        // Cerrar el archivo de registro
        fclose(fp);

        printf("Se ha agregado una nueva linea al archivo de registro.\n");
    }
}


sem_t sem1;
sem_t sem2;
sem_t sem3;

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

    FILE* fp = fopen(INVENTORY_FILE, "r");
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
        sem_post(&sem1);
        sem_post(&sem2);
    }

 fclose(fp);
    pthread_exit(NULL);
}

void* patron2(void* arg) {
    int tiempo_antiguo[MAX_USERS] = {0};
    int incremento = 10; // Tiempo en segundos para considerar incremento de apuestas
    char id_usuario[MAX_LINE_LENGTH];
    int total_apuestas;
    int tiempo_actual;

    FILE* fp = fopen(INVENTORY_FILE, "r");
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

        strcpy(id_usuario, tokens[3]);
        total_apuestas = atoi(tokens[4]);
        tiempo_actual = atoi(tokens[7]);

        sem_wait(&sem2);
        if (tiempo_actual - tiempo_antiguo[atoi(id_usuario)] < incremento) {

            printf("Se ha encontrado el patron 2 para el usuario con DNI: %s\n", id_usuario);

        }
        tiempo_antiguo[atoi(id_usuario)] = tiempo_actual;
        sem_post(&sem2);
        sem_post(&sem3);
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

    fp = fopen(INVENTORY_FILE, "r");
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

        }   sem_post(&sem3);
            sem_post(&sem1);
        // Añadir la apuesta al array de apuestas del usuario
        if (users[user_index].num_apuestas < MAX_USERS) {
            users[user_index].apuestas[users[user_index].num_apuestas] = total_apuestas;
            users[user_index].num_apuestas++;
        }
        if (fgets(line, MAX_LINE_LENGTH, fp) == NULL) {
                pthread_exit(NULL);
fclose(fp);
    }
    }
}

int main()
{
    leerfichero();
    FILE* file = fopen(INVENTORY_FILE, "r");
    if (!file)
    {
        printf("Error: no se puede abrir el archivo.\n");
        exit(1);
    }

    sem_init(&sem1, 0, 1); // Inicializar el semáforo del arreglo de patrones
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 0);

    pthread_t thread1,thread2, thread3;
printf("\nLOS LUDÓPATAS DETECTADOS HASTA EL MOMENTO SON\n");
    pthread_create(&thread1, NULL, patron1, (void*) file);
    pthread_create(&thread2, NULL, patron2, (void*) file);
    pthread_create(&thread3, NULL, patron3, (void*) file);


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    dormir();

    fclose(file);
    registrarInstancia(INVENTORY_FILE);
    exit(0);
}


