#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/inotify.h>
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

#define NUM_HILOS 3

sem_t semaforos[5];
sem_t semcinco;
sem_t general;
sem_t lectura;
sem_t cons;
sem_t escribir;
sem_t num_rows;
sem_t semlog;

char PATH_FILES[20];
char* INVENTORY_FILE;
char* LOG_FILE;
int NUM_PROCESOS;
int SIMULATE_SLEEP;
int CASAS_APUESTAS;
int SIMULATE_SLEEP_MIN;
int SIMULATE_SLEEP_MAX;

int num_output_rows=0;
int ant_output_rows=0;



typedef struct
{
    char id_apuesta[20];
    char fecha_inicio[20];
    char fecha_fin[20];
    char id_usuario[20];
    char id_sesion_juego[20];
    int participacion;
    float apuesta;
    float estado;
    char casa_apuesta[20];
} input_row;


typedef struct
{
    char casa_apuesta[20];
    char fecha_inicio[20];
    char fecha_fin[20];
    char id_usuario[20];
    char id_juego[20];
    int total_participaciones;
    float total_apuestas;
    float total_estado_pos;
    float total_estado_neg;
} output_row;

output_row output_rows [MAX_OUTPUT_ROWS];

typedef struct
{
    char *filename;
} read_thread_args;

typedef struct
{
    char *filename;
} consolidate_thread_args;

typedef struct
{
    char *filename;
} write_thread_args;

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
/*
void dormir() {
    int retardo = rand() % (SIMULATE_SLEEP_MAX - SIMULATE_SLEEP_MIN + 1) + SIMULATE_SLEEP_MIN;
    sleep(retardo);
}*/

//"ficheros/CA001_BET365_POKER_ON_30032023_1.csv"
void get_casa_apuesta(char* ruta, char* casa_apuesta)
{
    char* token = strtok(ruta, "/");
    strtok(NULL,"_");
    while (token != NULL)
    {
        token = strtok(NULL, "_");
        if (token != NULL)
        {
            strcpy(casa_apuesta, token);
            break;
        }
    }
}

void registrarInstancia(char* filename) {
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
    fp = fopen(filename, "r");

    // Contar el número de líneas en el archivo
    if (fp) {
        while ((c = getc(fp)) != EOF) {
            if (c == '\n') {
                no_apuestas_consolidadas++;
            }
        }
        fclose(fp);
    } else {
        printf("No se pudo abrir el archivo %s.\n", filename);
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
                filename, no_apuestas_consolidadas);
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
                filename, no_apuestas_consolidadas);
            // Contar el número de líneas en el archivo


        // Cerrar el archivo de registro
        fclose(fp);


    }
}


void funcion_consolidar(char* filename){
            // Aqu� puedes agregar el c�digo para
            //sleep(SIMULATE_SLEEP);
            FILE* file;
            file=fopen(filename,"r");

            sem_wait(&semlog);
            registrarInstancia(filename);
            sem_post(&semlog);

            input_row input_rows[MAX_INPUT_ROWS];
            int num_input_rows=0;
            sem_post(&general);
            if (!file)
            {
                printf("Error opening file %s\n",filename);
                exit(1);
            }

            char line[256];

            // Leemos la primera l�nea del archivo que contiene los nombres de los campos
            fgets(line, sizeof(line), file);
            // Iteramos por cada l�nea del archivo
            sem_wait(&lectura);
            char *casa_apuesta=(char*) malloc(sizeof(char)*MAX_CASA_APUESTA_LENGTH);
            get_casa_apuesta(filename,casa_apuesta);
            while (fgets(line, sizeof(line), file))
            {
                //Verificamos que la línea no esté vacía
                if(strlen(line)<=1){
                    continue;
                }
                // Utilizamos strtok para separar los campos de la l�nea
                char* field = strtok(line, ";");
                int i = 0;

                // Iteramos por cada campo de la l�nea y lo almacenamos en la estructura Bet
                while (field != NULL)
                {
                    switch (i)
                    {
                    case 0:
                        strcpy(input_rows [num_input_rows].id_apuesta, field);
                        break;
                    case 1:
                        strcpy(input_rows[num_input_rows].fecha_inicio, field);
                        break;
                    case 2:
                        strcpy(input_rows[num_input_rows].fecha_fin, field);
                        break;
                    case 3:
                        strcpy(input_rows[num_input_rows].id_usuario, field);
                        break;
                    case 4:
                        strcpy(input_rows[num_input_rows].id_sesion_juego, field);
                        break;
                    case 5:
                        input_rows[num_input_rows].participacion = atoi(field);
                        break;
                    case 6:
                        (input_rows[num_input_rows].apuesta = atoi(field));
                        break;
                    case 7:
                        input_rows[num_input_rows].estado = atoi(field);
                        break;
                    default:
                        break;
                    }

                    // Pasamos al siguiente campo de la l�nea
                    field = strtok(NULL, ";");
                    i++;
                }
                // Aqu� puedes utilizar la estructura Bet para lo que necesites
                strcpy(input_rows[num_input_rows].casa_apuesta,casa_apuesta);

                num_input_rows++;
            }
            fclose(file);

            sem_post(&lectura);
            sem_post(&cons);

            int i, j;
            output_row row;
            int found=0;
            sem_wait(&num_rows);
            ant_output_rows=num_output_rows;
            sem_post(&num_rows);
            //input rows necesita contador?
            for (i = 0; i < num_input_rows; i++)
            {

                found=0;
                for (j = 0; j < num_output_rows; j++)
                {
                    if (strcmp(input_rows[i].id_usuario, output_rows[j].id_usuario) == 0 &&
                            strcmp(input_rows[i].id_sesion_juego, output_rows[j].id_juego) == 0 &&
                            strcmp(input_rows[i].casa_apuesta,output_rows[j].casa_apuesta)==0)
                    {
                        // Update existing row
                        found = 1;

                        // Ordenar�a por fecha

                        // fecha1=input_rows[i].fecha_inicio
                        // fecha2=output_rows[j].fecha_inicio
                        // fecha3=input_rows[i].fecha_fin
                        // fecha4=output_rows[j].fecha_fin

                        int dia1, mes1, anio1, hora1, minuto1, segundo1;
                        int dia2, mes2, anio2, hora2, minuto2, segundo2;
                        int dia3, mes3, anio3, hora3, minuto3, segundo3;
                        int dia4, mes4, anio4, hora4, minuto4, segundo4;

                        // Convertir los strings en valores enteros
                        sscanf(input_rows[i].fecha_inicio, "%d/%d/%d %d:%d:%d", &dia1, &mes1, &anio1, &hora1, &minuto2, &segundo1);
                        sscanf(output_rows[j].fecha_inicio, "%d/%d/%d %d:%d:%d", &dia2, &mes2, &anio2, &hora2, &minuto2, &segundo2);
                        sscanf(input_rows[i].fecha_fin, "%d/%d/%d %d:%d:%d", &dia3, &mes3, &anio3, &hora3, &minuto3, &segundo3);
                        sscanf(output_rows[j].fecha_fin, "%d/%d/%d %d:%d:%d", &dia4, &mes4, &anio4, &hora4, &minuto4, &segundo4);

                        // Comparar los componentes de la fecha
                        if (anio1 < anio2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (anio1 > anio2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);

                        }
                        else if (mes1 < mes2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (mes1 > mes2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
                        }
                        else if (dia1 < dia2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (dia1 > dia2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
                        }
                        else if (hora1 < hora2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (hora1 > hora2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
                        }
                        else if (minuto1 < minuto2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (minuto1 > minuto2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
                        }
                        else if (segundo1 < segundo2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (segundo1 > segundo2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
                        }
                        else
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }

                        // Ahora para las FECHAS FINALES
                        if (anio3 < anio4)
                        {
                            strcpy(row.fecha_fin, output_rows[j].fecha_fin);
                        }
                        else if (anio3 > anio4)
                        {
                            strcpy(row.fecha_fin, input_rows[i].fecha_fin);;
                        }
                        else if (mes3 < mes4)
                        {
                            strcpy(row.fecha_fin, output_rows[j].fecha_fin);
                        }
                        else if (mes3 > mes4)
                        {
                            strcpy(row.fecha_fin, input_rows[i].fecha_fin);
                        }
                        else if (dia3 < dia4)
                        {
                            strcpy(row.fecha_fin, output_rows[j].fecha_fin);
                        }
                        else if (dia3 > dia4)
                        {
                            strcpy(row.fecha_fin, input_rows[i].fecha_fin);
                        }
                        else if (hora3 < hora4)
                        {
                            strcpy(row.fecha_fin, output_rows[j].fecha_fin);
                        }
                        else if (hora3 > hora4)
                        {
                            strcpy(row.fecha_fin, input_rows[i].fecha_fin);

                        }
                        else if (minuto1 < minuto2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (minuto1 > minuto2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
                        }
                        else if (segundo1 < segundo2)
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        else if (segundo1 > segundo2)
                        {
                            strcpy(row.fecha_inicio, output_rows[j].fecha_inicio);
                        }
                        else
                        {
                            strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                        }
                        row.total_participaciones = input_rows[i].participacion;
                        row.total_apuestas = output_rows[j].total_apuestas + input_rows[i].apuesta;

                        if (input_rows[i].estado > 0)
                        {
                            row.total_estado_pos = output_rows[j].total_estado_pos + input_rows[i].estado;
                        }
                        else
                        {
                            row.total_estado_neg = output_rows[j].total_estado_neg + input_rows[i].estado;
                        }
                        strcpy(row.casa_apuesta, input_rows[i].casa_apuesta);
                        strcpy(row.id_usuario,input_rows[i].id_usuario);
                        strcpy(row.id_juego,input_rows[i].id_sesion_juego);
                        sem_wait(&cons);
                        output_rows[j]= row;
                        sem_post(&cons);



                    }
                }

                if (!found)
                {
                    // Add new row
                    row.total_estado_neg=0;
                    row.total_estado_pos=0;
                    //get_casa_apuesta(filename,row.casa_apuesta);
                    strcpy(row.casa_apuesta,input_rows[i].casa_apuesta);
                    strcpy(row.fecha_inicio, input_rows[i].fecha_inicio);
                    strcpy(row.fecha_fin, input_rows[i].fecha_fin);
                    strcpy(row.id_usuario, input_rows[i].id_usuario);
                    strcpy(row.id_juego, input_rows[i].id_sesion_juego);
                    row.total_participaciones = input_rows[i].participacion;
                    row.total_apuestas = input_rows[i].apuesta;
                    if (input_rows[i].estado > 0)
                    {
                        row.total_estado_pos = input_rows[i].estado;
                    }
                    else
                    {
                        row.total_estado_neg = input_rows[i].estado;
                    }
                    sem_wait(&cons);
                    output_rows[j] = row;
                    sem_post(&cons);
                    sem_post(&escribir);

                    sem_wait(&num_rows);
                    num_output_rows++;
                    sem_post(&num_rows);


                }
            }

            //ESCRIBIR EN CONSOLIDATED.CSV

            strcpy(filename,INVENTORY_FILE);
            file = fopen(filename, "w");
            if (!file)
            {
                printf("Error opening file %s\n", filename);
                exit(1);
            }
            if(ant_output_rows==0)
            {
                fprintf(file, "CasaApuesta;FECHA_INICIO;FECHA_FIN;IdUsuario;IdJuego;TotalParticipaciones;TotalApuestas;TotalEstadoPos;TotalEstadoNeg\n");
            }


            sem_wait(&escribir);
            for (int i = 0; i < num_output_rows; i++)
            {
                fprintf(file, "%s;%s;%s;%s;%s;%d;%.2f;%.2f;%.2f\n",
                        output_rows[i].casa_apuesta,
                        output_rows[i].fecha_inicio,
                        output_rows[i].fecha_fin,
                        output_rows[i].id_usuario,
                        output_rows[i].id_juego,
                        output_rows[i].total_participaciones,
                        output_rows[i].total_apuestas,
                        output_rows[i].total_estado_pos,
                        output_rows[i].total_estado_neg);

            }

            fclose(file);
            sem_post(&escribir);
            //dormir();
            sem_wait(&semlog);
             system("./MONITOR");
            sem_post(&semlog);

            //METER AQUÍ CÓDIGO
            sem_wait(&general);
}

void* consolidar(void* arg)
{

    int num_casa=*((int*) arg);
    num_casa++; //Xk el hilo 0 se encargará de la casa de apuestas 1

    DIR *dir;
    struct dirent *ent;
    //char *path = PATH_FILES; // directorio actual

    char filename[MAX_FILENAME_LEN];
    char barra[]="/";
    char prefix[3];
    int number;


    dir = opendir(PATH_FILES);// abre el directorio

    //int fd;
    //fd=inotify_init();

    //if (fd<0){
    //  perror("inotify_init");
    //}

    if (dir == NULL)
    {
        perror("No se pudo abrir el directorio");
        exit(1);
    }

    while ((ent = readdir(dir)) != NULL)   // lee cada archivo en el directorio
    {

        char* endptr;
        number=strtol(ent->d_name + 2, &endptr, 10);
        if (*endptr == '_' && ent->d_type == DT_REG && num_casa==number)   // verifica si es un archivo regular
        {
            sem_wait(&general);

            //printf("\nEl archivo %s ha sido movido al directorio\n",ent->d_name);
            //sleep(SIMULATE_SLEEP);
            strcpy(filename,PATH_FILES);
            strcat(filename,barra);
            strcat(filename,ent->d_name);

            funcion_consolidar(filename);
            sem_post(&general);
        }
    }

    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];

    fd = inotify_init();

    if ( fd < 0 )
    {
        perror( "inotify_init" );
    }

    wd = inotify_add_watch( fd, PATH_FILES, IN_MOVED_TO );

    if ( wd < 0 )
    {
        perror( "inotify_add_watch" );
    }

    while ( 1 )
    {

        i = 0;
        length = read( fd, buffer, BUF_LEN );

        if ( length < 0 )
        {
            perror( "read" );
        }
        while ( i < length )
        {
            sem_wait(&general);
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
            if ( event->mask & IN_MOVED_TO )
            {
                char* endptr;
                number=strtol(event->name + 2, &endptr, 10);




                if (*endptr=='_' && num_casa==number)   // verifica si es un archivo regular
                {
                    //printf("El archivo %s ha sido movido al directorio",event->name);
                    //sleep(SIMULATE_SLEEP);
                    strcpy(filename,PATH_FILES);
                    strcat(filename,barra);
                    strcat(filename,event->name);
                    // Aqu� puedes agregar el c�digo para
                    funcion_consolidar(filename);
                    //METER AQUÍ CÓDIGO
                }
            }
            i += EVENT_SIZE + event->len;
            sem_post(&general);
        }

    }
    ( void ) inotify_rm_watch( fd, wd );
    ( void ) close( fd );
    exit(0);

    closedir(dir);

    // se encontraron archivos, esperar a que haya cambios
    //inotify_wait(fd);

    //Código antiguo

    //CONSOLIDAR
}



int main()
{
    leerfichero();
    pthread_t hilos[CASAS_APUESTAS]; // Limitamos a 5 hilos
    pthread_t hilosayuda[NUM_PROCESOS-CASAS_APUESTAS]; //Hilos de ayuda
    sem_init (&general, 0, 1);
    sem_init (&lectura, 0, 1);
    sem_init (&cons, 0, 0);
    sem_init (&escribir, 0, 0);
    sem_init (&num_rows, 0, 1);
    sem_init (&semlog, 0, 1);

    int thread_casas_nums[CASAS_APUESTAS];

    for(int i=0; i<CASAS_APUESTAS; i++){
        thread_casas_nums[i]=i;
    }

    DIR *dir;
    struct dirent *ent;
    //char *path = PATH_FILES; // directorio actual
    int num_casa;
    int *punt_casa=&num_casa;
    int encontro=0;
    //fd=inotify_init();

    //if (fd<0){
    //    perror("inotify_init");
    //}

    for(int k=0; k<CASAS_APUESTAS; k++){

        num_casa=k;
        pthread_create(&hilos[k], NULL,consolidar, (void*)&thread_casas_nums[k]);
        //sleep(SIMULATE_SLEEP);
    }
    for(int k=0; k<CASAS_APUESTAS; k++){
        pthread_join(hilos[k], NULL);

    }
    sem_destroy(&semlog);
    sem_destroy(&general);
return 0;
}
