#ifndef FUNCIONESCLIENTE_H_
#define FUNCIONESCLIENTE_H_
//PROTOCOLO v0.9
#define CLIENTE_ENVIA_COMPARTIDOS 0
#define CLIENTE_SOLICITA_COMPARTIDOS_Y_CLIENTES 1
#define CLIENTE_SOLICITA_DESCARGA 2
#define SERVIDOR_SOLICITA_ARCHIVO 3
#define CLIENTE_ENVIA_ARCHIVO_COMPARTIDO 4
#define RECIBIR_ARCHIVO_COMPARTIDO 5
#define SERVIDOR_ENVIA_LISTA_COMPARTIDOS 6
//END PROTOCOLO v0.1
#define CANTTHR 5 //TODO: a archivo de datos de configuración
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include "helper.h"
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

typedef struct fileCmp
{
    int cliente;
    char id[256];
    char nom[256];
    int tam;
    int cantidadDescargas;
    struct fileCmp *sig;
} arcCmp;

typedef struct sConectados
{
    int socketDescriptor;
    struct sConectados *sig;
} tipoConectados;

typedef struct argumentosComp
{
    int descriptorSocket;
    char carpetaEscogida[256];
    char rutaDescarga[256];
} argumentosThreads;

int leerDir(char carpetaEscogida[], int descriptorSocket);
int cantidadArchivosEnDirectorio(char carpetaEscogida[]);
int recibirCompartidosServidor(int src);
void insertarEnListaDeArchivosClientes(int cliente, char nombreDeArchivo[], int tamanioArchivo, char idArchivo[]);
void mostrarListaCompartidosServidor(arcCmp * copia);
int enviarOpcion (int opc, int descriptorSocket); //Envía la opción para que el servidor accione de acuerdo al cliente
int guardarListaCompartidosEnLog();
int enviarALog(int tipo, char *msg);
int enviarArchivoAlServidor(char rutaArchivoSolicitado[], int descriptorSocket, int clienteDestino); //preliminar
void *threadCompartirCarpeta(void *argsStr); //Por cada opción un thread distinto
void *threadEnviarDescargarArchivos(void *argStr);
char *crypt(const char *key, const char *salt);
void transferir(int srcfd, char *rutaDescarga);
int leerConfiguraciones();
#endif /* FUNCIONESCLIENTE_H_ */
