#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

void proteccionAntiCuelgue(int s)  //Protección para el padre
{
    printf("\nSoy el padre: %d. Recibí la protección anticuelgue\n", getpid());
    //TODO: Cierra el archivo.
    //TODO: Finaliza su ejecución.
    //TODO: Debe dejar constancia en el log
}

void escribirEnLog();

int main(int argc, char *argv[])
{
    int padre;
    padre = getpid();
    if(argc != 3)
    {
        perror("MODO DE USO: ./padre 5 /tmp/");
        exit(EXIT_FAILURE);
    }
    else
    {
        int i;
        int cantHijos;
        char nombreDir[256];
        cantHijos = atoi(argv[1]);
        strcpy(nombreDir, argv[2]);
        if(cantHijos < 1)
        {
            perror("MODO DE USO: ./padre X /tmp/, siendo X un numero mayor que 0.");
            exit(EXIT_FAILURE);
        }
        else
        {
            DIR *directorioRecibido;
            struct dirent *archivoActual;
            char bufferPipe[256]; //Buffer para leer y escribir del pipe
            int fdPipe[2]; //Pipe para los hijos pares
            if(pipe(fdPipe) == -1)
            {
                perror("ERROR:padre:pipe");
            }
            else
            {
                signal(SIGALRM, proteccionAntiCuelgue);
                alarm(10);
                int hijos[cantHijos];
                printf("\nSe ingresaron %d hijos y el directorio es %s\n", cantHijos, nombreDir);
                for(i = 0; i < cantHijos; i++)
                {
                    //Crea hijos
                    if((hijos[i] = fork()) == -1)
                    {
                        perror("error:padre:fork");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        if(hijos[i] == 0)  //Estoy en el hijo
                        {
                            directorioRecibido = opendir(nombreDir);
                            if((getpid()%2) == 0)
                            {
                                //El hijo es par
                                printf("\nEl hijo %d, pid: %d es par. Su padre es %d\n", i, getpid(), getppid());
                                //Enviar la cantidad de archivos del directorio dado
                                int cantidadDeArchivos = 0;
                                while((archivoActual = readdir(directorioRecibido)) != NULL)
                                {
                                    if((strcmp(archivoActual->d_name, ".") != 0) && (strcmp(archivoActual->d_name, "..")) != 0)
                                    {
                                        cantidadDeArchivos++;
                                    }
                                }
                                memset(bufferPipe, '\0', 256);
                                sprintf(bufferPipe, "%d", cantidadDeArchivos);
                                if(write(fdPipe[1], bufferPipe, 256) == -1)
                                {
                                    perror("ERROR:HIJO:FDPIPE1:WRITE");
                                    exit(EXIT_FAILURE);
                                }
                            }
                            else     //El hijo es impar
                            {
                                int fdPrimerArchivo; //File descriptor del primer archivo
                                printf("\nEl hijo %d, pid: %d es impar. Su padre es %d\n", i, getpid(), getppid());
                                while((archivoActual = readdir(directorioRecibido)) != NULL)
                                {
                                    int flagCortePrimerArchivo = 0;
                                    if((strcmp(archivoActual->d_name, ".") != 0) && (strcmp(archivoActual->d_name, "..")) != 0 && flagCortePrimerArchivo == 0)
                                    {
                                        strcat(nombreDir, archivoActual->d_name);
                                        //printf("\nEl primer archivo en el directorio es: %s\n", nombreDir);
                                        if((fdPrimerArchivo = open(nombreDir, O_RDONLY)) == -1)
                                        {
                                            perror("ERROR:HIJOIMPAR:OPEN");
                                        }
                                        else
                                        {
                                            int totalBytesLeidos = 0;
                                            int bytesLeidos = 0;
                                            char bufferArch[256];
                                            while((bytesLeidos = read(fdPrimerArchivo, bufferArch, 256)) > 0)
                                            {
                                                totalBytesLeidos = totalBytesLeidos + bytesLeidos;
                                            }
                                            //printf("\nSoy hijo impar el total de bytes leidos es: %d y se lo envío a mi padre\n", totalBytesLeidos);
                                            memset(bufferPipe, '\0', 256);
                                            sprintf(bufferPipe, "%d", totalBytesLeidos);
                                            if(write(fdPipe[1], bufferPipe, 256) == -1)
                                            {
                                                perror("ERROR:HIJOIMPAR:FDPIPE1:WRITE");
                                                exit(EXIT_FAILURE);
                                            }
                                        }
                                        flagCortePrimerArchivo = 1;
                                    }
                                }
                            }
                            closedir(directorioRecibido);
                            //TODO: Antes de morir los hijos escriben en el log
                            escribirEnLog();
                            exit(EXIT_SUCCESS);
                        }
                        else
                        {
                            int fd;
                            char buffer[256];
                            if((fd = open("padre.txt", O_CREAT|O_APPEND|O_WRONLY, 0777)) == -1) //ESCRIBO EL REGISTRO EN TODOS LOS CASOS; PAR O NO
                            {
                                perror("PADRE:OPEN:ESCRIBIRENREGISTRO");
                                exit(EXIT_FAILURE);
                            }
                            memset(buffer, '\0', 256);
                            //Estoy en el padre
                            if((hijos[i]%2) != 0)  //El proceso hijo es impar
                            {
                                memset(bufferPipe, '\0', 256);
                                read(fdPipe[0], bufferPipe, 256);
                                printf("\nUn hijo impar me envió la cantidad de caracteres del primer archivo en el directorio: %s\n", bufferPipe);
                                sprintf(buffer, "\nUn hijo impar me envió la cantidad de caracteres del primer archivo en el directorio: %s\n", bufferPipe);

                            }
                            if((hijos[i]%2) == 0)  //El proceso hijo es par
                            {
                                memset(bufferPipe, '\0', 256);
                                read(fdPipe[0], bufferPipe, 256);
                                printf("\nUn hijo par me envió la cantidad de archivos del directorio: %s\n", bufferPipe);
                                sprintf(buffer, "\nUn hijo par me envió la cantidad de archivos del directorio: %s\n", bufferPipe);
                            }
                            if((write(fd, buffer, strlen(buffer))) == -1) //ESCRIBO EL REGISTRO EN TODOS LOS CASOS; PAR O NO
                            {
                                perror("PADRE:WRITE:ESCRIBIRENREGISTRO");
                                exit(EXIT_FAILURE);
                            }
                            close(fd);
                        }
                    }
                } //FIN DEL FOR
                if(getpid() == padre)
                {
                    for(i = 0; i < cantHijos; i++)
                    {
                        wait(NULL);
                    }
                    //TODO: TENGO QUE DEJAR REGISTRO QUE FINALIZARON MIS HIJOS
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
}

void escribirEnLog()
{

    int fd;
    char buffer[256];
    if((fd = open("log.txt", O_CREAT|O_APPEND|O_WRONLY, 0777)) == -1)
    {
        perror("HIJO:OPEN:ESCRIBIRENLOG");
        exit(EXIT_FAILURE);
    }
    memset(buffer, '\0', 256);
    sprintf(buffer, "Soy el proceso pid: %d y muero.", getpid());
    if((write(fd, buffer, strlen(buffer))) == -1)
    {
        perror("HIJO:WRITE:ESCRIBIRENLOG");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

