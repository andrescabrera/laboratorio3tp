#include "funcionesCliente.h"

extern arcCmp *cabeza;
extern arcCmp *finalListaCompartidos;
extern char ipServidor[16];
//corre todo el tiempo, esperando que el servidor le pida un archivo
void *threadEnviarDescargarArchivos(void *argStr)
{
    argumentosThreads *argStrlocal = (argumentosThreads *) argStr;
    char bufferOpcion[64];
    char bufferRuta[256];
    char bufferDestino[64];
    int bytesLeidos;
    int opcionLeida;
    char mensaje[512];

    while(1)
    {
        if((bytesLeidos = read(argStrlocal->descriptorSocket, bufferOpcion, 64)) <= 0 )    //Leer opcion
        {
            printf("\nDesconectado.\n");
            enviarALog(0, "\nDesconectado\n");
            perror("ERROR:funcionesCliente.c:threadEnviarDescargarArchivos");
            exit(EXIT_FAILURE);
        }
        bufferOpcion[bytesLeidos] = '\0';
        sprintf(mensaje,"Lei la opcion %s.", bufferOpcion);
        enviarALog(0, mensaje);
        opcionLeida = atoi(bufferOpcion);
        if(opcionLeida == SERVIDOR_SOLICITA_ARCHIVO)
        {
            read(argStrlocal->descriptorSocket, bufferRuta, 256); //Leo la ruta //TODO: validar read
            bytesLeidos = read(argStrlocal->descriptorSocket, bufferDestino, 64);
            bufferDestino[bytesLeidos]='\0';
            sprintf(mensaje,"Soy el cliente, recibi la opcion: %d. Me pidieron %s, para enviar a %s.", opcionLeida, bufferRuta, bufferDestino);
            enviarALog(0, mensaje);
            if(enviarArchivoAlServidor(bufferRuta, argStrlocal->descriptorSocket, atoi(bufferDestino)) == -1)
            {
                //TODO: enviarOpcion(ENVIO_CANCELADO), opciones correctivas... NO-IMPLEMENTADO.
                enviarALog(0, "Error general al enviar el archivo solicitado, reintentando...");
            }
        }
        else if (opcionLeida == RECIBIR_ARCHIVO_COMPARTIDO)
        {
            //Transferir el archivo, también le paso la ruta del archivo.
            transferir(argStrlocal->descriptorSocket, ((argumentosThreads*)argStr)->rutaDescarga);
        }
        else if (opcionLeida == SERVIDOR_ENVIA_LISTA_COMPARTIDOS)
        {
            //Recibo la lista de compartidos desde el servidor.
            if(recibirCompartidosServidor(argStrlocal->descriptorSocket) == 0)
                enviarALog(0, "Se recibio la lista de compartidos correctamente.");
            guardarListaCompartidosEnLog();
        }
    }
}

int enviarArchivoAlServidor(char rutaArchivoSolicitado[], int descriptorSocket, int clienteDestino)
{
    struct stat bufstat;
    int descriptorArchivoSolicitado, bytesLeidos, bytesEscritos, totalBytesLeidos = 0, totalBytesEscritos;
    char buffer [256];
    char mensaje [512];

    if (enviarOpcion(CLIENTE_ENVIA_ARCHIVO_COMPARTIDO, descriptorSocket) == 1)  //fin de envio opcion
    {
        enviarALog(0, "ERROR:ENVIO:OPCIONENVIOARCHIVO");
        perror("ERROR:ENVIO:OPCIONENVIOARCHIVO");
        return EXIT_FAILURE;
    }
    if (enviarOpcion(clienteDestino, descriptorSocket) == 1)  //fin de envio opcion
    {
        enviarALog(0, "ERROR:ENVIO:CLIENTEDESTINO");
        perror("ERROR:ENVIO:CLIENTEDESTINO");
        return EXIT_FAILURE;
    }

    if (stat(rutaArchivoSolicitado, &bufstat) == -1 )
    {
        enviarALog(0, "ERROR:funcionesCliente.c:enviarArchivoAlServidor:stat:No pude saber el tamaño del archivo.");
        perror("ERROR:funcionesCliente.c:enviarArchivoAlServidor:stat:No pude saber el tamaño del archivo.");
        return EXIT_FAILURE;
    }
    //Envio el tamaño del archivo
    sprintf(buffer, "%ld", bufstat.st_size);
    if(write(descriptorSocket, buffer, 255) == -1)
    {
        enviarALog(0, "ERROR:funcionesCliente.c:enviarArchivoAlServidor:No pude enviar el tamaño.");
        perror("ERROR:funcionesCliente.c:enviarArchivoAlServidor:No pude enviar el tamaño.");
        return EXIT_FAILURE;
    }
    if((descriptorArchivoSolicitado = open(rutaArchivoSolicitado, O_RDONLY)) == -1)
    {
        enviarALog(0, "ERROR:funcionesCliente.c:enviarArchivoAlServidor:No se pudo abrir el archivo solicitado.");
        perror("ERROR:funcionesCliente.c:enviarArchivoAlServidor:No se pudo abrir el archivo solicitado.");
        return EXIT_FAILURE;
    }
    sprintf(mensaje,"Estoy enviando %s al servidor.", rutaArchivoSolicitado);
    enviarALog(0, mensaje);
    //leo el archivo y voy enviando por el socket
    while(totalBytesLeidos < bufstat.st_size)
    {
        memset(buffer, '\0', 256);
        if((bytesLeidos = read(descriptorArchivoSolicitado, buffer, 256)) == -1)
        {
            enviarALog(0, "ERROR:error al leer el archivo que debo enviar.");
            perror("ERROR:error al leer el archivo que debo enviar.");
            return EXIT_FAILURE;
        }
        if((bytesEscritos = write(descriptorSocket, buffer, bytesLeidos)) == -1)  //escribo socket
        {
            enviarALog(0, "ERROR:error al enviar el archivo desde el cliente.");
            perror("ERROR:error al enviar el archivo desde el cliente.");
            return EXIT_FAILURE;
        }
        totalBytesLeidos += bytesLeidos;
        totalBytesEscritos += bytesEscritos;
    }
    close(descriptorArchivoSolicitado);
    sprintf(mensaje,"Envio correcto: Se leyo del archivo %d bytes, y se envio al servidor %d bytes.", totalBytesLeidos, totalBytesEscritos);
    enviarALog(0, mensaje);
}

int leerDir(char carpetaEscogida[], int descriptorSocket)
{
    DIR *directorio;
    directorio = opendir(carpetaEscogida);	// directorio a abrir
    if (directorio == NULL)
    {
        enviarALog(0, "No se puede acceder al directorio.");
        return EXIT_FAILURE;
    }
    else
    {
        struct dirent *directorioActual;
        struct stat datosDelArchivo;
        int cantidadDeArchivos = -1;
        char stCurrentDir[256];
        strcpy(stCurrentDir, carpetaEscogida);
        strcat(stCurrentDir, "/");
        char stFile[256];
        char bufferNombreArchivo[256];
        char bufferTamanio[64];
        //envio cantidad
        int bytesEscritos; //cantidad escrita
        cantidadDeArchivos = cantidadArchivosEnDirectorio(carpetaEscogida);
        if(cantidadDeArchivos == -1)
            return EXIT_FAILURE;
        memset(bufferTamanio,'\0', 64);
        sprintf(bufferTamanio, "%u", cantidadDeArchivos);
        bytesEscritos = write(descriptorSocket, bufferTamanio, sizeof(bufferTamanio));
        //fin de envio cantidad
        while ((directorioActual = readdir(directorio)) != NULL)  	//loop, avanza solo...
        {
            if ((strcmp(directorioActual->d_name, ".") != 0) && (strcmp(directorioActual->d_name, "..") != 0))
            {
                strcpy(stFile, stCurrentDir);
                strcat(stFile, directorioActual->d_name);
                stat(stFile, &datosDelArchivo);
                if (!S_ISDIR(datosDelArchivo.st_mode))
                {
                    memset(bufferNombreArchivo,'\0', 256);
                    memset(bufferTamanio,'\0', 64);
                    sprintf(bufferNombreArchivo, "%s", stFile);
                    sprintf(bufferTamanio, "%lu", datosDelArchivo.st_size);
                    bytesEscritos = write(descriptorSocket, bufferNombreArchivo, sizeof(bufferNombreArchivo));
                    bytesEscritos = write(descriptorSocket, bufferTamanio, sizeof(bufferTamanio));
                }
            }
        }
        closedir(directorio);
        return 0;
    }
}

int cantidadArchivosEnDirectorio(char carpetaEscogida[])
{
    DIR *directorio;
    directorio = opendir(carpetaEscogida);	// directorio a abrir
    if (directorio == NULL)
    {
        enviarALog(0, "No se puede acceder al directorio.");
        return EXIT_FAILURE;
    }
    else
    {
        struct dirent *directorioActual;
        struct stat datosDelArchivo;
        int cantidadDeArchivos = 0;
        char stCurrentDir[256];
        strcpy(stCurrentDir, carpetaEscogida);
        strcat(stCurrentDir, "/");
        char stFile[256];
        while ((directorioActual = readdir(directorio)) != NULL)  	//loop, avanza solo...
        {
            if ((strcmp(directorioActual->d_name, ".") != 0) && (strcmp(directorioActual->d_name, "..") != 0))
            {
                strcpy(stFile, stCurrentDir);
                strcat(stFile, directorioActual->d_name);
                stat(stFile, &datosDelArchivo);
                if (S_ISDIR(datosDelArchivo.st_mode))
                {
                }
                else
                {
                    cantidadDeArchivos++;
                }
            }
        }
        closedir(directorio);
        return cantidadDeArchivos;
    }
    return EXIT_FAILURE;
}

//Recibo la lista de los archivos que hay en el servidor
int recibirCompartidosServidor (int srcfd)
{
    char bufferCliente[8];
    char buffNom[256];
    char buffId[256];
    char buffTam[64];
    char registroEnLog[256];
    int cnt;
    int i = 0, n = 0;
    //leo la cantidad - 64bytes
    if ((cnt = read(srcfd, buffTam, 64)) > 0)
        n = atoi(buffTam);		//cantidad de archivos a recibir
    sprintf(registroEnLog, "Me estan enviando la lista de compartidos del servidor. Tiene %d archivos.", n);
    enviarALog(0, registroEnLog);
    if (n > 0)
    {
        //cargo en la primera pos. lo primero que recibo
        if((cnt = read(srcfd, bufferCliente, 8)) > 0)
            if ((cnt = read(srcfd, buffNom, 256)) > 0)
                if ((cnt = read(srcfd, buffTam, 64)) > 0)
                    if ((cnt = read(srcfd, buffId, 256)) > 0) //TODO: Ver tema tamaño del ID
                        insertarEnListaDeArchivosClientes(atoi(bufferCliente), buffNom, atoi(buffTam), buffId);
        //y avanzo con el resto
        while (i < n - 1)
        {
            if((cnt = read(srcfd, bufferCliente, 8)) > 0)
                if ((cnt = read(srcfd, buffNom, 256)) > 0)
                    if ((cnt = read(srcfd, buffTam, 64)) > 0)
                        if ((cnt = read(srcfd, buffId, 256)) > 0) //TODO: Ver tema tamaño del ID
                            insertarEnListaDeArchivosClientes(atoi(bufferCliente), buffNom, atoi(buffTam), buffId);
            i++;
        }
    }
    return 0;
}

//Inserta al final de una lista de archivos compartidos de los archivos del servidor
void insertarEnListaDeArchivosClientes(int cliente, char nombreDeArchivo[], int tamanioArchivo, char idArchivo[])
{
    arcCmp *nuevoArchivoCompartido = (arcCmp *) malloc(sizeof(arcCmp));
    strcpy(nuevoArchivoCompartido->nom, nombreDeArchivo);
    strcpy(nuevoArchivoCompartido->id, idArchivo);
    nuevoArchivoCompartido->cliente = cliente;
    nuevoArchivoCompartido->tam = tamanioArchivo;
    nuevoArchivoCompartido->sig = NULL;
    if (finalListaCompartidos != NULL)
    {
        finalListaCompartidos->sig = nuevoArchivoCompartido; //Engancho el ultimo con el nuevo.
        finalListaCompartidos = finalListaCompartidos->sig; //Avanzo el final de lista porque agregue uno
    }
    else     //Si es el primero de la lista es el ultimo.
    {
        finalListaCompartidos = nuevoArchivoCompartido;
        cabeza = finalListaCompartidos;
    }
}

//Funciones para Threads
void *threadCompartirCarpeta(void *argsStr)
{
    char mensaje[512];
    argumentosThreads *argStrlocal = (argumentosThreads *) argsStr;

    if (enviarOpcion(CLIENTE_ENVIA_COMPARTIDOS, argStrlocal->descriptorSocket) == 1)  //fin de envio opcion
    {
        enviarALog(0, "ERROR:ENVIO:OPCION");
        perror("ERROR:ENVIO:OPCION");
        exit(EXIT_FAILURE);
    }

    sprintf(mensaje,"El nombre de la carpeta dentro del thread es:%s", argStrlocal->carpetaEscogida);
    enviarALog(0, mensaje);
    if(leerDir(argStrlocal->carpetaEscogida, argStrlocal->descriptorSocket) != 0)  //funcion leer los directorios de la carpeta
    {
        enviarALog(0, "leerDir:cantidadArchivosEnDirectorio:no se pudo recuperar");
        perror("leerDir:cantidadArchivosEnDirectorio:no se pudo recuperar");
        exit(EXIT_SUCCESS);
    }
}

int enviarOpcion (int opc, int descriptorSocket)
{
    char buffer[64];
    int bytesEscritos;
    memset(buffer,'\0', 64);
    sprintf(buffer, "%d", opc);
    if((bytesEscritos = write(descriptorSocket, buffer, 64)) < 0)
        return 1;
    else
        return 0;
}

int guardarListaCompartidosEnLog()
{
    char buffer[256];
    arcCmp *regArc = cabeza;
    while (regArc != NULL)
    {
        memset(buffer, '\0', 256);
        sprintf(buffer, "Cliente: %d", regArc->cliente);
        enviarALog(0, buffer);
        memset(buffer, '\0', 256);
        sprintf(buffer, "\t FileName: %s\t", regArc->nom);
        enviarALog(0, buffer);
        sprintf(buffer, "Tamaño: %d\t", regArc->tam);
        enviarALog(0, buffer);
        sprintf(buffer, "Cantidad de Descargas: %d\t", regArc->cantidadDescargas);
        enviarALog(0, buffer);
        sprintf(buffer, "Id: %s\n", regArc->id);
        enviarALog(0, buffer);
        regArc = regArc->sig;
    }
    return 0;
}

//Muestro la lista de los archivos compartidos en el servidor
void mostrarListaCompartidosServidor(arcCmp * copia)
{
    enviarALog(0, "\n\t\tMostrando la lista.\n\n");
    arcCmp *regArc = cabeza;
    while (regArc != NULL)
    {
        printf("\nFileName: %s ", regArc->nom);
        printf("Tamaño: %d ", regArc->tam);
        printf("En cliente: %d ", regArc->cliente);
        printf("Cantidad de Descargas: %d\n", regArc->cantidadDescargas);
        regArc = regArc->sig;
    }
}

int enviarALog(int tipo, char *msg)
{
    int logFileDescriptor;
    char buffer[1024];
    char nombreLog[256];
    //fecha
    time_t curtime;
    struct tm *lt;
    curtime = time(NULL);
    lt = localtime(&curtime);
    if(tipo == 0)
    {
        //el nombre del log se forma con el pid
        sprintf(nombreLog, "./logs/logCliente%d.log", getpid());
        sprintf(buffer, "%d-%d-%d %d-%d-%d - [%ld]: %s\n", lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, pthread_self(), msg);
    }
    else if (tipo == 1)
    {
        //La lista de compartidos se guarda en un archivo aparte.
        sprintf(nombreLog, "./logs/logListaCompartidos%d.log", getpid());
        sprintf(buffer, "%s", msg);
    }
    else
    {
        //Cualquier otra tipo que se envie es incorrecto.
        return EXIT_FAILURE;
    }
    //Abro el log
    if((logFileDescriptor = open(nombreLog, O_CREAT|O_APPEND|O_WRONLY, 0600)) == -1)
    {
        enviarALog(0, "ERROR:enviarALog:Error al abrir el archivo");
        perror("ERROR:enviarALog:Error al abrir el archivo");
        return EXIT_FAILURE;
    }
    //Escribo en log
    if(write(logFileDescriptor, buffer, strlen(buffer)) == -1)
    {
        perror("ERROR:enviarALog:Problema al escribir log.");
        enviarALog(0, "ERROR:enviarALog:Problema al escribir log.");
        close(logFileDescriptor);
        return EXIT_FAILURE;
    }
    close(logFileDescriptor);
    return 0;
}

void transferir(int srcfd, char *rutaDescarga)
{
    int tamanioArchivoARecibir;
    int fdToDisk;			//descriptor para archivo en disco
    int cantidadLeida, cantidadEscrita;
    int totalBytesLeidos = 0, totalBytesEscritos = 0;
    char buf[256];			//tam del buffer
    char mensaje[512];
    //parseo la ruta
    sprintf(mensaje, "La ruta de la descarga es: %s", rutaDescarga);
    enviarALog(0, mensaje);
    //me voy hasta el barra
    char nombreArchivo[256];
    char nuevaRuta[512];
    int i = 256, j = 0;
    while(rutaDescarga[i] != '/')
    {
        i--;
    }
    i++; //avanzo el /
    //guardo el nuevo nombre
    while(rutaDescarga[i] != '\0')
    {
        nombreArchivo[j] = rutaDescarga[i];
        i++;
        j++;
    }
    nombreArchivo[j] = '\0';
    sprintf(nuevaRuta, "./downloads/%s", nombreArchivo);
    sprintf(mensaje, "El nuevo nombre de archivo es %s", nuevaRuta);
    enviarALog(0, mensaje);
    //fecha
    time_t curtime;
    struct tm *lt;
    curtime = time(NULL);
    lt = localtime(&curtime);

    if((cantidadLeida = read(srcfd, buf, 255)) == -1)
    {
        enviarALog(0, "ERROR:helper.c:transferir:Error al leer el tamaño del archivo a recibir.");
        exit(EXIT_FAILURE);
    }
    buf[cantidadLeida]='\0';
    tamanioArchivoARecibir = atoi(buf);
    sprintf(mensaje,"Me estan enviando un archivo de %d bytes", tamanioArchivoARecibir);
    enviarALog(0, mensaje);

    if ((fdToDisk = open(nuevaRuta, O_CREAT | O_WRONLY, 0777)) == -1)
    {
        enviarALog(0, "Error al guardar el archivo destino");
        exit(EXIT_FAILURE);
    }
    //Copio el archivo desde el socket
    while (totalBytesLeidos < tamanioArchivoARecibir)
    {
        if((cantidadLeida = read(srcfd, buf, 256)) == -1)
        {
            enviarALog(0, "ERROR:helper.c:transferir:read:Error al leer del socket.");
        }
        if ((cantidadEscrita = write(fdToDisk, buf, cantidadLeida)) != cantidadLeida)
        {
            enviarALog(0, "ERROR:helper.c:transferir:write:Error al escribir el archivo a disco.");
            exit(EXIT_FAILURE);
        }
        totalBytesLeidos += cantidadLeida;
        totalBytesEscritos += cantidadEscrita;
    }
    sprintf(mensaje, "Termine de recibir el archivo. Se leyo %d bytes y se escribo en el archivo %d bytes.", totalBytesEscritos, totalBytesLeidos);
    enviarALog(0, mensaje);
    close(fdToDisk);
}

int leerConfiguraciones()
{
    int fd, i = 0;
    char buffer;
    char mensaje[256];
    if((fd = open("configuraciones.txt", O_RDONLY)) == -1)
    {
        perror("ERROR: Error al leer del archivo de configuraciones.");
        enviarALog(0, "ERROR: Error al leer del archivo de configuraciones.");
        return -1;
    }
    while(buffer != ';')
    {
        if(read(fd, &buffer, 1) == -1)
        {
            perror("ERROR: Error al leer del archivo de configuraciones.");
            enviarALog(0, "ERROR: Error al leer del archivo de configuraciones.");
            return -1;
        }
        if(buffer != ';')
        {
            ipServidor[i] = buffer;
            i++;
        }
    }
    ipServidor[i] = '\0';
    sprintf(mensaje, "Se leyo la ip del servidor, es: %s.", ipServidor);
    enviarALog(0, mensaje);
    return 0;
}
