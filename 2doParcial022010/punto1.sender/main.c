#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#define TIPO_MSG_HI 11

typedef struct mensaje
{
    long tipoMensaje;
    int idProceso;
} mensajeCola;

int rannm(int a, int b);

int main(int argc, char *argv[])
{
    mensajeCola mensaje;
    int llaveMsg;
    int msgId;
    int i;
    int cantidadMsgEnviar;
    char tipoSender[20];
    if (argc != 3)
    {
        perror("\nERROR: Modo de uso: ./nombreBinario --all o --hi xx (cantidad de mensajes a enviar)");
        exit(EXIT_FAILURE);
    }
    strcpy(tipoSender,argv[1]);
    cantidadMsgEnviar = atoi(argv[2]);

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
    for(i=0; i<=cantidadMsgEnviar; i++)
    {
        if (strcmp(tipoSender,"--all")==0)
            mensaje.tipoMensaje=rannm(1,10);
        else if (strcmp(tipoSender,"--hi")==0)
            mensaje.tipoMensaje=TIPO_MSG_HI;

        mensaje.idProceso = getpid();
        if(msgsnd(msgId, &mensaje, sizeof(mensaje)-sizeof(mensaje.tipoMensaje), 0) == -1)
        {
            perror("Error al enviar el mensaje.");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);

}

int rannm(int a, int b)
{
    static int ret;
    if(ret++ == 0)
    {
        srand(time(NULL));
    }
    return (rand()%(b-a+1))+a;
}

