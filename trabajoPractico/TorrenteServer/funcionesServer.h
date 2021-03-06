#ifndef FUNCIONESSERVER_H_
#define FUNCIONESSERVER_H_
//PROTOCOLO v0.9
#define CLIENTE_ENVIA_COMPARTIDOS 0
#define CLIENTE_SOLICITA_COMPARTIDOS_Y_CLIENTES 1
#define CLIENTE_SOLICITA_DESCARGA 2
#define SERVIDOR_SOLICITA_ARCHIVO 3
#define CLIENTE_ENVIA_ARCHIVO_COMPARTIDO 4
#define RECIBIR_ARCHIVO_COMPARTIDO 5
#define SERVIDOR_ENVIA_LISTA_COMPARTIDOS 6
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include "helper.h"
#include <linux/unistd.h>

typedef struct fileCmp
{
    int cliente;
    char id[256];
    char nom[256];
    int tam;
    int cantidadDescargas;
    struct fileCmp *sig;
} arcCmp;

typedef struct thargs
{
    double *array;
    int descriptorCliente;
    int size;
} strarg;

typedef struct sConectados
{
    int socketDescriptor;
    struct sConectados *sig;
} tipoConectados;

int leerConfiguraciones();
void insertarCliente(int socketDescriptor);
int recibirCompartidos(int srcfd);
char *crypt(const char *key, const char *salt);
void *fc_threads(void *argumentos);
void mostrarListaCompartidos();
void insertarEnLista(int cliente, char nombreDeArchivo[], int tamanioArchivo, char idArchivo[]);
int enviarListaCompartidos(arcCmp * copia, int cliente);
int guardarListaCompartidosEnLog();
void copiarArchivo(int clienteRecibe, int clienteEnvia);
int enviarArchivoCliente(int clienteRecibe, char idArchivo[]);
int buscarArchivo(char idArchivo[]);
int enviarOpcion (int opc, int descriptorSocket); //Envía la opción para que el cliente accione de acuerdo al servidor
int cantidadDeClientes();
void mostrarListaConectados();
int ipFiltrada(char *ipConectada);
#endif /* FUNCIONESSERVER_H_ */
