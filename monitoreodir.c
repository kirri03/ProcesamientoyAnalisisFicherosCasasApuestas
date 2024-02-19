#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int main( )
{
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];

    fd = inotify_init();

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
                // Aquí puedes agregar el código para procesar el archivo de datos de la casa de apuestas
            }
            i += EVENT_SIZE + event->len;
        }
    }

    ( void ) inotify_rm_watch( fd, wd );
    ( void ) close( fd );

    exit( 0 );
}
