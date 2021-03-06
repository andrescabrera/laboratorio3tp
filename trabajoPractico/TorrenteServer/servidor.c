/*
 * Servidor.
 * Recibe por ahora la lista de archivos compartidos de un solo cliente.
 * TODO: ver si conviene manejar una lista por cada cliente:
 * tener un array de listas de compartidos, de tamaño del maximo de conexiones que se reciban.
 */
#include "funcionesServer.h"

void menu();
void lanzador(int opcionMenu);
void *fc_administracion(void *argumentos);

arcCmp *cabeza;
tipoConectados *clientesConectados;
tipoConectados *finalClientesConectados;
arcCmp *finalListaCompartidos;
int cantidadDescargasTotales;
int cantidadDeConexiones;
char arrayIpsFiltradas[256][16];

int main(int argc, char *argv[])
{
    leerConfiguraciones();
    cantidadDescargasTotales = 0;
    pthread_t unThread;
    pthread_attr_t attr;
    char mensaje[64]; //Para enviar mensajes al log
    if (pthread_attr_init(&attr) != 0)
    {
        perror("Hubo un problema"), exit(-1);
    }
    int descriptor_socket, descriptorArchivoDatos;	// Descriptores de archivo de  los sockets
    struct sockaddr_in struc_servidor;	// Estructura de socket del servidor
    socklen_t tamanio_direccion;	// Tamaño de struc_servidor
    cabeza = NULL;
    clientesConectados = NULL;
    finalListaCompartidos = NULL;
    finalClientesConectados = NULL;
    /* Crear el Socket */
    if ((descriptor_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Inicicalizar y establecer la estructura del servidor */
    memset(&struc_servidor, 0, sizeof(struc_servidor));
    struc_servidor.sin_family = AF_INET;
    struc_servidor.sin_port = htons(50000);

    /* Ligar el socket a una direccion de red */
    tamanio_direccion = sizeof(struc_servidor);
    if ((bind(descriptor_socket, (struct sockaddr *) &struc_servidor, tamanio_direccion)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /* Quedar a la espera de conexiones entrantes */
    if ((listen(descriptor_socket, cantidadDeConexiones)) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    enviarALog("Bienvenido a Torrente v0.2\tBuild 0012");
    enviarALog("Socket TCP/IP disponible");
    sprintf(mensaje,"\t Puerto: %d, Direccion IP: %s", ntohs(struc_servidor.sin_port), inet_ntoa(struc_servidor.sin_addr));
    enviarALog(mensaje);
    if (pthread_create(&unThread, &attr, fc_administracion, NULL) != 0) //Creo un thread para mostrar el menu del servidor
    {
        perror("Hubo un problema al crear un thread administración");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        if (cantidadDeClientes() < cantidadDeConexiones)
        {
            if ((descriptorArchivoDatos = accept(descriptor_socket, (struct sockaddr *) &struc_servidor, &tamanio_direccion)) >= 0)
            {
                sprintf(mensaje, "Nueva conexión desde ip: %s", inet_ntoa(struc_servidor.sin_addr));
                enviarALog(mensaje);
                if(ipFiltrada(inet_ntoa(struc_servidor.sin_addr)) == 0)
                {
                    insertarCliente(descriptorArchivoDatos); //Guardo los socket descriptors en una lista
                    strarg *argumentos;
                    argumentos = (strarg *) calloc(1, sizeof(strarg));	//le doy espacio en memoria
                    argumentos->descriptorCliente = descriptorArchivoDatos;
                    sprintf(mensaje, "El descriptor en el main es: %d", descriptorArchivoDatos);
                    enviarALog(mensaje);
                    if (pthread_create(&unThread, &attr, fc_threads, (void *) argumentos) != 0)
                    {
                        enviarALog("Hubo un problema al crear un thread");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    enviarALog("La ip estaba filtrada.");
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}

void *fc_administracion(void *argumentos)  //Thread en paralelo para la parte de Administración.
{
    menu();
    return NULL;
}

//Menú Administrativo
void menu()
{
    int opcionMenu;					//opción del menú

    while(1)
    {
        printf("\n\tSeleccione la opción que desee: \n");
        printf("\t1 - Visualizar la cantidad de clientes online\n");
        printf("\t2 - Visualizar la cantidad de archivos descargados\n");
        printf("\t3 - Visualizar la cantidad de descargas de los archivos compartidos.\n");
        printf("\t4 - Visualizar listado de clientes conectados\n");
        printf("\t5 - Desconectar a un cliente.\n");

        scanf("%d", &opcionMenu);

        if (opcionMenu < 1 || opcionMenu > 6)
        {
            system("clear");
            printf("\n\n\tLa opción del menu es inexistente.");
        }
        else
        {
            lanzador(opcionMenu);
        }
    }
}

//Lanzador de Funciones de Administración
void lanzador(int opcionMenu)
{
    //TODO: NOTA: Para las primeras cuatro opciones: pueden ser mostradas todas en una misma pantalla.
    int cliente;
    switch (opcionMenu)
    {
    case 1:
        printf("\nHay %d clientes conectados en este momento.\n", cantidadDeClientes());
        break;
    case 2:
        printf("Se descargaron %d archivos hasta el momento.\n", cantidadDescargasTotales);
        break;
    case 3:
        printf("Muestro la cantidad de descargas de los archivos compartidos.");
        mostrarListaCompartidos();
        break;
    case 4:
        printf("Muestro el listado de clientes conectados.");
        mostrarListaConectados();
        break;
    case 5:
        printf("Ingrese un numero de cliente para desconectarlo: ");
        scanf("%d", &cliente);
        close(cliente);
        borrarClienteDeLaLista(cliente);
        eliminarCliente(cliente);
        break;
    }
}
