#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <string.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


#define MAX_FILENAME_LEN 100
#define MAX_LINE_LEN 70
#define MAX_INPUT_ROWS 1000
#define MAX_OUTPUT_ROWS 10000
#define MAX_CASA_APUESTA_LENGTH 10
#define MAX_CASAS_APUESTAS 10

#define NUM_HILOS 3


char PATH_FILES[20];
char* INVENTORY_FILE;
char* LOG_FILE;
int NUM_PROCESOS;
int SIMULATE_SLEEP;
int CASAS_APUESTAS;
int SIMULATE_SLEEP_MIN;
int SIMULATE_SLEEP_MAX;

char* DIRECTORIO;
char* DIRECTORIOS_CASAS[MAX_CASAS_APUESTAS];

int num_output_rows=0;
int ant_output_rows=0;

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
        else if (strcmp (name, "DIRECTORIO") == 0){
            DIRECTORIO = strdup(value);
            strcat(DIRECTORIO,"/");
        }
        else if (strcmp (name, "DIRECTORIOS_CASAS")==0){
            char* field= strtok(value, ",");
            for(int i=0; i<CASAS_APUESTAS;i++){
                DIRECTORIOS_CASAS[i]=malloc(MAX_FILENAME_LEN * sizeof(char));
                strcpy(DIRECTORIOS_CASAS[i],DIRECTORIO);
                strcat(DIRECTORIOS_CASAS[i],field);
                field=strtok(NULL,",");
            }
        }
    }

    // Cierra el archivo de configuraci�n
    fclose(conf_file);

}

int main(){
    leerfichero();
    printf("%s,%s,%d,%d,%d,%d,%d,%s,%s,%s,%s,%s",INVENTORY_FILE,LOG_FILE, NUM_PROCESOS, SIMULATE_SLEEP, CASAS_APUESTAS, SIMULATE_SLEEP_MIN, SIMULATE_SLEEP_MAX, DIRECTORIO, DIRECTORIOS_CASAS[0],DIRECTORIOS_CASAS[1],DIRECTORIOS_CASAS[2],DIRECTORIOS_CASAS[3]);

  // Crea el directorio principal si no existe
    mkdir(DIRECTORIO, 0777);

    // Crea los directorios de las casas de apuestas
    for (int i=0;i<CASAS_APUESTAS;i++){
        mkdir(DIRECTORIOS_CASAS[i], 0777);
    }

    return 0;
}
