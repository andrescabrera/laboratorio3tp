#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct mensaje
{
    long tipoMensaje;
    int idProceso;
} mensajeCola;

int main(int argc, char *argv[])
{
    int llaveMsg;
    int msgId;
    int cantidadMsgEnviar;
    cantidadMsgEnviar = atoi(argv[1]);
    if (argc != 2)
    {
        perror("\nERROR: CANT ARGUMENTOS NO VALIDA");
        exit(EXIT_FAILURE);
    }

    if((llaveMsg = ftok("/bin/ls", 'q')) == -1)
    {
        perror("Error al obtener llaveMsg");
        exit(EXIT_FAILURE);
    }
    if((msgId = msgget(llaveMsg, IPC_CREAT | IPC_EXCL | 0777)) == -1)
    {
        perror("Error al obtener la cola.");
        exit(EXIT_FAILURE);
    }


}
