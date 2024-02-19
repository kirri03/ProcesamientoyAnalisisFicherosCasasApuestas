#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 10000 //Tamaño máximo del archivo csv

typedef struct {
    char casa_apuesta[20];
    char fecha_inicio[20];
    char fecha_fin[20];
    char id_usuario[20];
    int id_juego;
    int total_participaciones;
    int total_apuestas;
    int total_estado_pos;
    int total_estado_neg;
}apuesta;

//Función para leer el archivo csv y almacenar los datos en una estructura apuesta
int leer_csv(apuesta apuestas[]) {
    FILE* fp = fopen("consolidated.csv", "r");
    if (fp == NULL) {
        printf("No se pudo abrir el archivo\n");
        return -1;
    }
    char linea[1000];
    int i = 0;
    while (fgets(linea, MAX_SIZE, fp) != NULL) {
        char* token = strtok(linea, ",");
        strcpy(apuestas[i].casa_apuesta, token);
        token = strtok(NULL, ",");
        strcpy(apuestas[i].fecha_inicio, token);
        token = strtok(NULL, ",");
        strcpy(apuestas[i].fecha_fin, token);
        token = strtok(NULL, ",");
        strcpy(apuestas[i].id_usuario, token);
        token = strtok(NULL, ",");
        apuestas[i].id_juego = atoi(token);
        token = strtok(NULL, ",");
        apuestas[i].total_participaciones = atoi(token);
        token = strtok(NULL, ",");
        apuestas[i].total_apuestas = atoi(token);
        token = strtok(NULL, ",");
        apuestas[i].total_estado_pos = atoi(token);
        token = strtok(NULL, ",");
        apuestas[i].total_estado_neg = atoi(token);
        i++;
	printf("%s\n",apuestas[i].casa_apuesta);
    }
    fclose(fp);
    return i;
}

//Función para detectar ludópatas a partir de tres tendencias
void detectar_ludopatas(apuesta apuestascons[], int num_apuestas) {
    int i, j, k;
    for (i = 0; i < num_apuestas; i++) {
        int total_participaciones = apuestascons[i].total_participaciones;
        int total_apuestas = apuestascons[i].total_apuestas;
        int total_ganancias = apuestascons[i].total_estado_pos - apuestascons[i].total_estado_neg;
        if (total_ganancias < -0.5 * total_apuestas && total_apuestas > 100 && total_participaciones > 10) {
            printf("El usuario %s es un posible ludópata\n", apuestascons[i].id_usuario);
        }
        for (j = i + 1; j < num_apuestas; j++) {
            if (strcmp(apuestascons[i].id_usuario, apuestascons[j].id_usuario) == 0) {
                total_participaciones += apuestascons[j].total_participaciones;
                total_apuestas += apuestascons[j].total_apuestas;
                total_ganancias += apuestascons[j].total_estado_pos - apuestascons[j].total_estado_neg;
                if (total_ganancias < -0.5 * total_apuestas && total_apuestas > 100 && total_participaciones > 10) {
                    printf("El usuario %s es un posible ludópata\n", apuestascons[i].id_usuario);
                    break;
                }
            }
        }
    }
}

int main(){
	apuesta apuestas[MAX_SIZE];
	int num_apuestas=0;
	num_apuestas=leer_csv(apuestas);
	printf("%i",num_apuestas);
	detectar_ludopatas(apuestas, num_apuestas);
	return 0;
}
