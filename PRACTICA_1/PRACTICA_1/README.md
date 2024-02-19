EXPLICACIÓN DE LA SOLUCIÓN:
En esta práctica se nos propone crear un pool de procesos para la detección de ludopatía de distintos usuarios, para ello hemos de leer los ficheros que las casas de apuestas nos envían aleatoriamente a lo largo de un tiempo infinito y procesarlos para extraer a los usuarios que presentan signos de ludopatía.
Desglose del problema:
Para abordar este problema tenemos que tener en cuenta muchos aspectos, por tanto, hemos decidido que lo mejor antes de ponerse a programar era desglosar el problema en partes:
El primer problema que se nos plantea es el “File Processor”. Un proceso que llevaba a cabo la automatización de la consolidación de los archivos que entran a la carpeta “ficheros”, es decir, tenemos que crear un proceso que fuera capaz de estar ejecutándose de forma contínua pero teniendo en cuenta que sólamente tendría que actuar si se incluía a la carpeta de ficheros algún archivo para así procesarlo.

El segundo problema es el proceso “Monitor”. Este, se encarga de leer el archivo que el “File Processor” ha creado, el archivo consolidado; Una vez leído, se encarga de buscar los patrones escogidos como patrones para la detección de ludopatía; en nuestro caso, hemos escogido los siguientes:
Patrón 1: el tiempo de sesión diaria del usuario (tiempo de apuesta) aumenta en un período corto de tiempo.
Patrón 2: el usuario participa en varios tipos de apuestas.
Patrón 3:  la cantidad del dinero apostado se incrementa en períodos cortos de tiempo.

Una vez creado el pool de procesos y el monitor, es necesario conectarlos entre sí para que cuando se consolide un archivo, el proceso “File Processor”, se comunique con el monitor mediante una tubería para así analizar lo consolidado previamente.

Tras la conexión de estos procesos, los procesos principales, hay que crear el fichero log, un fichero en el cual estarían registrados cada una de las operaciones de impresión en el fichero consolidado como de lectura en el mismo.ç

Además del fichero log, es necesario crear un archivo de configuración para la lectura e inicialización de variables en los procesos para que al modificarlo se modifique todo el código para las especificaciones de las variables.

Tras crear todo lo necesario, tenemos que utilizar semáforos en cada uno de los procesos principales y procesos ligeros para así garantizar la sincronización y la concurrencia.
Explicación detallada:
File Processor: 
File Processor al empezar su ejecución lee el fichero de configuración para setear las variables y crea la estructura del directorio especificada en este mismo archivo con los permisos necesarios.
Después lanza un hilo por cada casa de apuestas y cada uno comprobará si hay más de un fichero en su directorio utilizando la librería dirent.h. Este hilo seguirá comprobando por si llega algún fichero nuevo a su directorio utilizando la librería inotify.h y un bucle infinito. En caso de que haya solo un hilo en el directorio se lanzará un hilo y si hay más de un fichero en el directorio se lanzarán N hilos.  Cada hilo leerá los ficheros que correspondan con su casa de apuestas asignada y los irán consolidado en la memoria compartida. Utilizaremos un bucle que hará una iteración por cada uno de los archivos que haya en este directorio, donde comprobaremos si se  trata de un archivo regular antes de leer el fichero

Lo siguiente sería llamar al hilo “funcion_consolidar” pasándole como argumento la ruta y nombre del archivo. Esta función abrirá esta ruta con permiso de lectura la cual utilizando semáforos, pasará a la función “registrar iteración” para que introduzca en el registro LOG.log los datos del archivo, la función “funcion_consolidar”, leerá línea por línea guardando cada campo en el array de structs “input_rows”, que es una variable global de esta función e irá aumentando el valor de “num_input_rows”, que se inicializa en 0. Después comprobará recorriendo el array de structs “input_rows”, que contiene la información acerca de las líneas que tiene el fichero que se está procesando y comparará los campos “casa_apuesta”, “id_usuario” e “id_sesion_juego” con estos mismos campos del array de structs “output_rows” que es una variable global que guarda los datos que se mostrarán en el consolidado. Si estos campos coinciden, entonces se consolidarán en esa línea del “output_rows”, cogiendo primera fecha inicial y la última fecha final, siendo la participación la última introducida y sumando el total de apuestas, el total de los estados positivos y el total de los estados negativos. Si no coinciden se agregará una nueva fila que tendrá los mismos campos que la fila de “input_rows”.

Por último, esta función accede a la memoria compartida creada en el main para introducir los valores de output_rows dentro de ella de tal forma que se crea un array de estructuras comportándose así como el antiguo archivo consolidado.

Monitor:
El monitor es el encargado de procesar los datos alojados en la memoria creada por el File Processor mediante el uso de hilos, los resultados de los patrones escogidos se guardarán en una segunda memoria compartida para comunicarse con el File Processor el cual imprimirá por pantalla dichos resultados. Para ello hemos de definir dichos patrones como funciones además de la función del fichero log que se explicará en el siguiente punto y una estructura que nos servirá de apoyo para un patrón.

Para el primero de los patrones, hemos de buscar si un usuario aumenta cada vez más el tiempo que dedica a apostar siempre que las participaciones totales sean superiores a 5, de forma independiente al juego al que esté jugando. Para conseguir esto, tenemos que recorrer la memoria compartida creada por el File Processor para cada usuario y si tiene más de una sesión de juego, comparar el tiempo que dedica a esa misma sesión, si ese tiempo aumenta, se imprimirá por pantalla que se ha detectado el patrón 1 para ese usuario en concreto. Después de eso, pasará a la siguiente línea y volverá a repetir el proceso. 
	
Para el segundo patrón, buscamos aquellos usuarios que en los cuales la cantidad de dinero apostado aumenta en un breve periodo de tiempo, para ello hemos diseñado un programa que mediante un proceso ligero compara el tiempo actual con el tiempo almacenado previamente en un array llamado “tiempo antiguo” para el usuario con el ID especificado previamente en “id_usuario”. Si la diferencia entre el tiempo actual y el tiempo almacenado es menor que “incremento” en relación a la cantidad de dinero apostado, entonces imprime un mensaje que indica que se ha encontrado el patrón 2 para el usuario indicado. Después de eso, se eliminan los datos del array y continúa a la siguiente línea y así de forma sucesiva.

Para el tercer y último patrón, buscamos a los usuarios que en un periodo corto de tiempo la cantidad apostada incrementa. Para comprobarlo, como en cada patrón primero recorreremos el archivo consolidado para cada usuario, si un usuario tiene más de una sesión, entonces podremos empezar a buscar dicho patrón. Para comprobar si se cumple el patrón en este caso, tenemos que comparar la cantidad de dinero apostado, es decir, comprobar que la variable “TotalApuesta” es mayor que en la sesión anterior, para llevarlo a cabo, primero comprobamos que no se ha procesado previamente el usuario, si no lo ha hecho lo guarda en un array que más tarde se leerá para no volver a procesar el mismo usuario, tras eso, guardamos en la estructura de apoyo el id del usuario, el número de apuestas y las apuestas. Si el usuario tiene más de una apuesta realizada es entonces cuando empezaremos a comparar; para comparar y así introducir en la segunda memoria compartida aquellos usuarios que cumplen el patrón, guardamos la apuesta actual en una variable y la apuesta de la siguiente iteración en otra variable. Si esta es mayor que la apuesta anterior y va en aumento a lo largo del tiempo, se guardará en la memoria compartida que es ludópata al cumplir el tercer patrón.

Para garantizar la sincronización y que no se de problemas concurrencia(violación de segmento), este proceso implementa tres hilos y protege las secciones críticas con un mutex, de tal forma que no se puede escribir por pantalla a la vez los usuarios que cumplen los patrones, asegurándonos de esta manera que no existan problemas de concurrencia o al accede a memoria.

La función principal (monitor) es un proceso no ligero que se encarga de extraer la información de la memoria compartida creada por File Processor y de crear una segunda memoria compartida en la que se declararán los usuarios encontrados como ludópatas, una vez hecho esto, procedemos a declarar los semáforos necesarios, los cuales se inicializan en 1, en 0 y en 0. Tras la inicialización de los semáforos, se definen los hilos para su posterior creación. Es entonces cuando creamos los hilos necesarios, los cuales se crean a partir de las funciones previamente declaradas para los patrones, de tal forma que se cierra el primer semáforo el cual estaba inicializado en 1, se crea el primer hilo y se abre el segundo semáforo; tras esta segunda apertura, se vuelve a cerrar el segundo semáforo,  se crea el segundo hilo y después de su creación se abre el tercer semáforo. Después, se cierra de nuevo el tercer semáforo, se crea el tercer y el último hilo que ejecutará la búsqueda del tercer patrón abriéndose tras su creación el primero de los semáforos para que se ejecute el primer patrón de nuevo. Una vez controlada la sincronización con los semáforos, se destruyen cada uno de los hilos con la función “pthread_join”, se llama a la función dormir, que con los datos SIMULATE_SLEEP_MIN y SIMULATE_SLEEP_MAX extraídos del fichero de configuración crea un sleep con un tiempo aleatorio.

Fichero LOG:
La función LOG es una función que está declarada tanto en el “File Processor” como en el “Monitor” puesto que es una función que registra en un archivo .log cada vez que se accede a un archivo, imprimiendo la fecha y la hora, el número del archivo que se ha procesado, el tiempo que se ha tardado en procesar dicho archivo, la ruta del archivo al que se ha accedido y las filas que han sido procesadas. Para llevar esto a cabo, a esta función habrá que pasarle la dirección del archivo. Una vez esta tenga dicha dirección, accede hasta el archivo y empieza a procesar siguiendo los siguientes pasos:
Declaración de variables necesarias
Apertura del archivo en modo lectura y lectura del mismo recorriendolo para contar las líneas, de tal forma que ya tenemos una variable para imprimir.
Verificación de existencia de un archivo LOG: no se requiere que se cree un log por cada archivo, por tanto si ya existe uno, se imprime en ese, en cambio, si no existe, se crea uno nuevo.
Una vez creado o si ya estaba previamente, se abre en modo escritura el archivo, si es de nueva creación, se imprimirá en el archivo una línea de guía y una línea más abajo, la línea con la información detallada de cada requerimiento de la línea de guía. Si no es de nueva creación, se imprimirá únicamente la línea con la información requerida.
Esta función se ha de llamar cada vez que se abre un archivo, por tanto se llamará en cada patrón y cada vez que se consolide un archivo nuevo. 

Archivo de configuración:
El archivo de configuración es el archivo en el que están las variables que el usuario que ejecuta el proceso, cambiará para modificar la creación del código, por tanto, es necesario extraer dichas variables inicializadas para conseguir que la modificación sea efectiva. 

Para ello, tenemos que leer e inicializar dichas variables en cada código que sea necesario, entre estas  están la ruta a los ficheros, la ruta al archivo consolidado, la ruta al fichero log, el número de procesos que se van a ejecutar, la variable SIMULATE_SLEEP_MAX y SIMULATE_SLEEP_MIN para ejecutar un retardo aleatorio, la localización del directorio, los directorios de las casas de apuestas los tamaños de las memorias y el número máximo de hilos.

	CASAS_APUESTAS indica el máximo número de apuestas.
	

Para hacerlo posible, hay dos opciones, crear una función a la que llamamos en la función principal o crear la lectura y seteo en la propia función principal. En nuestro caso hemos escogido la primera, crear una función y llamarla en la principal, ya que ofrece un mejor rendimiento. El código ejecutado en la función empezaría por la apertura del archivo de configuración en modo lectura, de este, se le asignan a cada una de las variables o rutas que existen en dicho archivo una variable en el proceso en el que se está ejecutando que han sido previamente declaradas, garantizando así que si se cambia el contenido de alguna variable del fichero de configuración afecte a todo el proceso.









  






ESQUEMA GENERAL:




En este esquema general podemos observar cómo se comportan los procesos creados, según los pasos:

Se define el archivo de configuración y se incluye en la carpeta ficheros.

Se incluyen en la carpeta los archivos enviados por las casas de apuestas.

Estos archivos son procesados por el File Processor creando un archivo consolidado y un archivo log cada vez que procesa un archivo.

El  File Processor se conecta con el monitor por medio de una tubería, el cual avisa a este cuando actualiza o crea el archivo consolidado.

El monitor, procesa el archivo consolidado e imprime por pantalla los usuarios que presentan signos de ludopatía, registrando cada vez que procesa dicho archivo en el fichero log.
