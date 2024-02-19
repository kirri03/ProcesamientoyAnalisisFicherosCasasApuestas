#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


#define MAX_FILENAME_LEN 100
#define MAX_LINE_LEN 17
#define MAX_INPUT_ROWS 1000
#define MAX_OUTPUT_ROWS 1000

typedef struct{
    char id_apuesta[20];
    char fecha_inicio[20];
    char fecha_fin[20];
    char id_usuario[20];
    char id_sesion_juego[20];
    int participacion;
    float apuesta;
    float estado;
}input_row;

typedef struct {
    char casa_apuesta[20];
    char fecha_inicio[20];
    char fecha_fin[20];
    char id_usuario[20];
    char id_juego[20];
    int total_participaciones;
    float total_apuestas;
    float total_estado_pos;
    float total_estado_neg;
}output_row;
//"ficheros/CA001_BET365_POKER_ON_30032023_1.csv"
void get_casa_apuesta(char* ruta, char* casa_apuesta) {
    char* token = strtok(ruta, "/");
    strtok(NULL,"_");
    while (token != NULL) {
        token = strtok(NULL, "_");
        if (token != NULL) {
            strcpy(casa_apuesta, token);
            break;
        }
    }
}



void read_input_file(char* filename, input_row bet[], int *num_rows) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[256];

    // Leemos la primera lï¿½nea del archivo que contiene los nombres de los campos
    fgets(line, sizeof(line), file);
    int j=*num_rows;
    // Iteramos por cada lï¿½nea del archivo
    while (fgets(line, sizeof(line), file))
    {
        // Utilizamos strtok para separar los campos de la lï¿½nea
        char* field = strtok(line, ";");
        int i = 0;

        // Iteramos por cada campo de la lï¿½nea y lo almacenamos en la estructura Bet
        while (field != NULL)
        {
            switch (i)
            {
            case 0:
		 strcpy(bet[j].id_apuesta, field);
                break;
            case 1:
                strcpy(bet[j].fecha_inicio, field);
                break;
            case 2:
                strcpy(bet[j].fecha_fin, field);
                break;
            case 3:
                strcpy(bet[j].id_usuario, field);
                break;
            case 4:
                strcpy(bet[j].id_sesion_juego, field);
                break;
            case 5:
                bet[j].participacion = atoi(field);
                break;
            case 6:
                bet[j].apuesta = atoi(field);
                break;
            case 7:
                bet[j].estado = atoi(field);
                break;
            default:
                break;
            }

            // Pasamos al siguiente campo de la lï¿½nea
            field = strtok(NULL, ";");
            i++;
        }

        // Aquï¿½ puedes utilizar la estructura Bet para lo que necesites
        printf("Bet ID: %s, Fecha inicio: %s, Fecha fin: %s, Usuario ID: %s, Sesion Juego ID: %s, Participacion: %d, Apuesta: %.2f, Estado: %.2f,\n", bet[j].id_apuesta, bet[j].fecha_inicio, bet[j].fecha_fin, bet[j].id_usuario, bet[j].id_sesion_juego, bet[j].participacion, bet[j].apuesta, bet[j].estado);
        j++;
        (*num_rows)++;
    }
    fclose(file);
}


void consolidate_data(input_row input_rows[], int *num_input_rows,
                      output_row output_rows[], int *num_output_rows, char *filename, int *ant_output_rows) {
    int i, j;
    output_row row;
    int found=0;
	*ant_output_rows+=*num_output_rows;
    for (i = 0; i < *num_input_rows; i++) {
        printf("\n\nIteraciï¿½n de i %i\n\n",i);
        found=0;
        for (j = 0; j <= *num_output_rows; j++) {
            if (strcmp(input_rows[i].id_usuario, output_rows[j].id_usuario) == 0 &&
                strcmp(input_rows[i].id_sesion_juego, output_rows[j].id_juego) == 0)
                {
                // Update existing row
                found = 1;
                // Ordenarï¿½a por fecha

    // fecha1=input_rows[i].fecha_inicio
    // fecha2=output_rows[j].fecha_inicio
    // fecha3=input_rows[i].fecha_fin
    // fecha4=output_rows[j].fecha_fin

    int dia1, mes1, anio1, hora1, minuto1, segundo1;
    int dia2, mes2, anio2, hora2, minuto2, segundo2;
    int dia3, mes3, anio3, hora3, minuto3, segundo3;
    int dia4, mes4, anio4, hora4, minuto4, segundo4;

    // Convertir los strings en valores enteros
    sscanf(input_rows[i].fecha_inicio, "%d/%d/%d %d:%d:%d", &dia1, &mes1, &anio1, &hora1, &minuto1, &segundo1);
    sscanf(output_rows[j].fecha_inicio, "%d/%d/%d %d:%d:%d", &dia2, &mes2, &anio2, &hora2, &minuto2,&segundo2);
    sscanf(input_rows[i].fecha_fin, "%d/%d/%d %d:%d:%d", &dia3, &mes3, &anio3, &hora3, &minuto3, &segundo3);
    sscanf(output_rows[i].fecha_fin, "%d/%d/%d %d:%d:%d", &dia4, &mes4, &anio4, &hora4, &minuto4, &segundo4);

    // Comparar los componentes de la fecha
    if (anio1 < anio2) {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (anio1 > anio2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);

    } else if (mes1 < mes2) {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (mes1 > mes2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
    } else if (dia1 < dia2) {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (dia1 > dia2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
    } else if (hora1 < hora2) {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (hora1 > hora2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
    } else if (minuto1 < minuto2) {
       strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (minuto1 > minuto2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
    } else if (segundo1 < segundo2) {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (segundo1 > segundo2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
    } else {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    }
    // Ahora para las FECHAS FINALES
    if (anio3 < anio4) {
         strcpy(row.fecha_fin, output_rows[j].fecha_fin);
    } else if (anio3 > anio4) {
        strcpy(row.fecha_fin, input_rows[i].fecha_fin);;
    } else if (mes3 < mes4) {
         strcpy(row.fecha_fin, output_rows[j].fecha_fin);
    } else if (mes3 > mes4) {
        strcpy(row.fecha_fin, input_rows[i].fecha_fin);
    } else if (dia3 < dia4) {
         strcpy(row.fecha_fin, output_rows[j].fecha_fin);
    } else if (dia3 > dia4) {
        strcpy(row.fecha_fin, input_rows[i].fecha_fin);
    } else if (hora3 < hora4) {
         strcpy(row.fecha_fin, output_rows[j].fecha_fin);
    } else if (hora3 > hora4) {
        strcpy(row.fecha_fin, input_rows[i].fecha_fin);

  } else if (minuto1 < minuto2) {
       strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (minuto1 > minuto2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
    } else if (segundo1 < segundo2) {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    } else if (segundo1 > segundo2) {
        strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
    } else {
        strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
    }
        row.total_participaciones = input_rows[i].participacion;
        row.total_apuestas = output_rows[j].total_apuestas + input_rows[i].apuesta;

        if (input_rows[i].estado > 0) {
            row.total_estado_pos = output_rows[j].total_estado_pos + input_rows[i].estado;
        } else {
            row.total_estado_neg = output_rows[j].total_estado_neg + input_rows[i].estado;
        }
        get_casa_apuesta(filename,row.casa_apuesta);
        strcpy(row.id_usuario,input_rows[i].id_usuario);
        strcpy(row.id_juego,input_rows[i].id_sesion_juego);
        output_rows[j] = row;
        printf("\n\n\n");
        printf("%s;%s;%s;%s;%s;%d;%.2f;%.2f;%.2f\n",
        row.casa_apuesta,
        row.fecha_inicio,
        row.fecha_fin,
        row.id_usuario,
        row.id_juego,
        row.total_participaciones,
        row.total_apuestas,
        row.total_estado_pos,
        row.total_estado_neg);

    }
            }

        if (!found) {
            // Add new row
            row.total_estado_neg=0;
            row.total_estado_pos=0;
            get_casa_apuesta(filename,row.casa_apuesta);
            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
            strcpy(row.fecha_fin, input_rows[i].fecha_fin);
            strcpy(row.id_usuario, input_rows[i].id_usuario);
            strcpy(row.id_juego, input_rows[i].id_sesion_juego);
            row.total_participaciones = input_rows[i].participacion;
            row.total_apuestas = input_rows[i].apuesta;
            if (input_rows[i].estado > 0) {
                row.total_estado_pos = input_rows[i].estado;
            } else {
                row.total_estado_neg = input_rows[i].estado;
            }

            output_rows[*num_output_rows] = row;
            (*num_output_rows)++;
            printf("\n\n%s;%s;%s;%s;%s;%d;%.2f;%.2f;%.2f\n",
                row.casa_apuesta,
                row.fecha_inicio,
                row.fecha_fin,
                row.id_usuario,
                row.id_juego,
                row.total_participaciones,
                row.total_apuestas,
                row.total_estado_pos,
                row.total_estado_neg);
    }
    }
                      }

void write_output_file(char* filename, output_row rows[], int *num_rows, int *ant_output_rows) {
    FILE* file = fopen(filename, "a");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }
	if(*ant_output_rows==0){
    fprintf(file, "CasaApuesta;FECHA_INICIO;FECHA_FIN;IdUsuario;IdJuego;TotalParticipaciones;TotalApuestas;TotalEstadoPos;TotalEstadoNeg\n");
				}    
for (int i = *ant_output_rows; i < *num_rows; i++) {
        fprintf(file, "%s;%s;%s;%s;%s;%d;%.2f;%.2f;%.2f\n",
                rows[i].casa_apuesta,
                rows[i].fecha_inicio,
                rows[i].fecha_fin,
                rows[i].id_usuario,
                rows[i].id_juego,
                rows[i].total_participaciones,
                rows[i].total_apuestas,
                rows[i].total_estado_pos,
                rows[i].total_estado_neg);
    }

fclose(file);   
}


int main( )
{
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];

    fd = inotify_init();

    char input_filename[MAX_FILENAME_LEN];
    int num_input_files = 1;
    char output_filename[MAX_FILENAME_LEN] = "consolidated.csv";
    input_row bet[MAX_INPUT_ROWS];
    int num_input_rows = 0;
    output_row output_rows[MAX_OUTPUT_ROWS];
    int num_output_rows = 0;
	int ant_output_rows=0;
	char carpeta[]="ficheros/";


    if ( fd < 0 ) {
        perror( "inotify_init" );
    }

    wd = inotify_add_watch( fd, "ficheros", IN_MOVED_TO );

    if ( wd < 0 ) {
        perror( "inotify_add_watch" );
    }

    while ( 1 ) {
        i = 0;
        length = read( fd, buffer, BUF_LEN );

        if ( length < 0 ) {
            perror( "read" );
        }
        while ( i < length ) {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
            if ( event->mask & IN_MOVED_TO ) {
                printf( "El archivo %s ha sido movido al directorio\n", event->name );
		strcpy(input_filename,carpeta);
		strcat(input_filename,event->name);
                // Aquí puedes agregar el código para procesar el archivo de datos de la casa de apuestas
		 read_input_file(input_filename, bet, &num_input_rows);
    		printf("\n\n");
    		consolidate_data(bet, &num_input_rows, output_rows, &num_output_rows, input_filename, &ant_output_rows);
    		write_output_file(output_filename, output_rows, &num_output_rows, &ant_output_rows);

            }
            i += EVENT_SIZE + event->len;
        }
    }

    ( void ) inotify_rm_watch( fd, wd );
    ( void ) close( fd );

    exit( 0 );
}

