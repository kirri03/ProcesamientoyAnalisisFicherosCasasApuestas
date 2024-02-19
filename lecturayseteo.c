#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#define MAX_LINE_LENGTH 256

// Definimos variables globales
char* PATH_FILES;
char* INVENTORY_FILE;
char* LOG_FILE;
int NUM_PROCESOS;
int SIMULATE_SLEEP;

void leerfichero() {
    // Abre el archivo de configuración
    FILE* conf_file = fopen("config.conf", "r");
    if (conf_file == NULL) {
        printf("No se pudo abrir el archivo de configuración\n");
        exit(1);
    }

    // Lee y procesa cada línea de configuración
    char buffer[MAX_LINE_LENGTH];
    char* name;
    char* value;
    while (fgets(buffer, MAX_LINE_LENGTH, conf_file) != NULL) {
        // Ignora las líneas en blanco o que comiencen con #
        if (buffer[0] == '\n' || buffer[0] == '#') {
            continue;
        }

        // Busca el signo = para separar el nombre de la variable y su valor
        name = strtok(buffer, "=");
        value = strtok(NULL, "=");

        // Elimina los espacios en blanco alrededor del valor
        value[strcspn(value, "\n")] = 0;
        while (*value == ' ') {
            ++value;
        }
        char* end = value + strlen(value) - 1;
        while (end > value && *end == ' ') {
            --end;
        }
        *(end + 1) = 0;

        // Configura las variables correspondientes
        if (strcmp(name, "PATH_FILES") == 0) {
            // Configura la variable PATH_FILES con el valor leído
            PATH_FILES = strdup(value);
        } else if (strcmp(name, "INVENTORY_FILE") == 0) {
            // Configura la variable INVENTORY_FILE con el valor leído
            INVENTORY_FILE = strdup( value);
        } else if (strcmp(name, "LOG_FILE") == 0) {
            // Configura la variable LOG_FILE con el valor leído
            LOG_FILE = strdup(value);
        } else if (strcmp(name, "NUM_PROCESOS") == 0) {
            // Configura la variable NUM_PROCESOS con el valor leído
            NUM_PROCESOS= atoi(value);
            
        } else if (strcmp(name, "SIMULATE_SLEEP") == 0) {
            // Configura la variable SIMULATE_SLEEP con el valor leído
            SIMULATE_SLEEP = atoi(value);
            
        }
    }

    // Cierra el archivo de configuración
    fclose(conf_file);

}

void* thread_function(void *arg){
	int thread_num = *(int*)arg;
	printf("Hilo %d ejecutÃ¡ndose", thread_num);
	//codigo hace hilo
	return NULL;
}
int main(){
	leerfichero();
	printf("%s,%s,%s,%d,%d",PATH_FILES,INVENTORY_FILE,LOG_FILE,NUM_PROCESOS,SIMULATE_SLEEP);
	pthread_t threads[NUM_PROCESOS];

	// Creamos cada hilo
}
