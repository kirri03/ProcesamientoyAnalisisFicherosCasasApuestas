#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 1000

void busqueda_ludopata(char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("No se puede abrir el archivo.\n");
        return;
    }

    char ludopatas[10][MAX_USERS];
    int num_ludopatas = 0;

    char line[1024];
	char id_usuario[10];
    while (fgets(line, 1024, file))
    {
        char* token = strtok(line, ";");
        int i = 0;
        char* fields[8];
        while (token != NULL)
        {
            fields[i++] = token;
            token = strtok(NULL, ";");
        }

	strcpy(id_usuario,fields[3]);
        if (id_usuario == 0)
        {
            continue;
        }

        // Comprobar si el usuario ya ha sido registrado
        int ludopata = 0;
        for (int i = 0; i < num_ludopatas; i++)
        {
            if (ludopatas[i] == id_usuario)
            {
                ludopata = 1;
                break;
            }
        }

        if (!ludopata)
        {
            // Comprobar si el usuario cumple con algun patron de ludopatia
            int participaciones = atoi(fields[5]);
            int apuestas = atoi(fields[6]);
            int ganancias = atoi(fields[7]);
            int perdidas = atoi(fields[8]);

            if (participaciones > 5)
            {
                // Patron 1: Un usuario que juega con demasiada frecuencia, por ejemplo, más de una vez al día durante un período prolongado de tiempo.
               num_ludopatas++;
		 strcpy(ludopatas[num_ludopatas],id_usuario);
            }
            else if (apuestas > 50 && (ganancias - perdidas) < -10)
            {
                // Patron 2: Un usuario que apuesta grandes cantidades de dinero en un corto período de tiempo.
               num_ludopatas++;
		 strcpy(ludopatas[num_ludopatas],id_usuario);
            }
            else if ((ganancias - perdidas) < -100)
            {
                // Patron 3: Un usuario que tiene más pérdidas que ganancias a lo largo del tiempo.
                num_ludopatas++;
		strcpy(ludopatas[num_ludopatas] , id_usuario);
            }
        }

        // Imprimir la lista de ludopatas encontrados
        printf("Usuarios con signos de ludopatia:\n");
        for (int i = 0; i < num_ludopatas; i++)
        {
            printf("%s\n", ludopatas[i]);
        }
    }
    fclose(file);
}

int main() {
busqueda_ludopata("consolidated.csv");
return 0;
}
