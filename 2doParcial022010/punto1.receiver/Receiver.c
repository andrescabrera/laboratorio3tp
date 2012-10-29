#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#define TIPO_HIJO 11
#define TIPO_PADRE 0

typedef struct mensaje
{
    long tipoMensaje;
    int idProceso;
} mensajeCola;

int main(int argc, char *argv[])
{
    mensajeCola mensaje;
    int llaveMsg, msgId;
    int receiver11;
    int tipoMsgLeer;
    int cantidadMensajesRecibidos = 1;
    if((llaveMsg = ftok("/bin/ls", 'q')) == -1)
    {
        perror("Error al obtener llaveMsg");
        exit(EXIT_FAILURE);
    }
    if((msgId = msgget(llaveMsg, IPC_CREAT|0777)) == -1)
    {
        perror("Error al obtener la cola.");
        exit(EXIT_FAILURE);
    }
    printf("Proceso padre: %d\n",getpid());
    if((receiver11 = fork()) == -1)
    {
        perror("Error: no pude forkear el receiver11");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        if (receiver11==0)
        {
            //soy el hijo,lee el tipo 11
            //printf("\nSoy el hijo: %d, Leo mensajes tipo 11",getpid());
            tipoMsgLeer=TIPO_HIJO;
        }
        else
        {
            // soy el padre, leo el resto de los mensajes
            //printf("\nSoy el padre: %d, leo el resto de los mensajes",getpid());
            tipoMsgLeer=TIPO_PADRE;
        }
        if(msgrcv(msgId, &mensaje, sizeof(mensaje)-sizeof(mensaje.tipoMensaje), tipoMsgLeer, IPC_NOWAIT) == -1)
        {
            if(errno != ENOMSG)
            {
                perror("\nError al recibir de la cola");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            printf("\nSoy pid: %d - Mensaje nro %d - tipo de mensaje %ld - enviado por el proceso %d", getpid(), cantidadMensajesRecibidos++, mensaje.tipoMensaje, mensaje.idProceso);
        }
    }
}
