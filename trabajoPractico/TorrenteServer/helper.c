#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include "helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

typedef struct thargs {
	int *array;
	int size;
} strarg;

void err_quit(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}
void herr_quit(char *msg) {
	herror(msg);
	exit(EXIT_FAILURE);
}
void xfer_data(int srcfd, int tgtfd) {
	char buf[1024];
	int cnt, len = 0;

	while ((cnt = read(srcfd, buf, sizeof(buf))) > 0) {
		if (len < 0)
			err_quit("helper.c:xfer_data:read");
		if ((len = write(tgtfd, buf, cnt)) != cnt)
			err_quit("helper.c:xfer_data:write");
	}
}

void *copiarSockets(void *argumentos) {
	char buf[1024];				//tam del buffer
	int cnt, len = 0;
	strarg *args;
	int *sdarray;
	args = (strarg *) argumentos;
	sdarray = args->array;

	while ((cnt = read(sdarray[0], buf, sizeof(buf))) > 0) {
		if (len < 0)
			err_quit("helper.c:xfer_data:read");
		if ((len = write(sdarray[1], buf, cnt)) != cnt)
			err_quit("helper.c:xfer_data:write");
	}
}

void transferir(int srcfd) {
	int fdToDisk;				//descriptor para archivo en disco
	char destino[20];			//nombre del archivo en srv
	char buf[1024];				//tam del buffer
	int cnt, len = 0;
	//fecha
	time_t curtime;
	struct tm *lt;
	curtime = time(NULL);
	lt = localtime(&curtime);

	sprintf(destino, "archivoCliente-%d-%d-%d-%d.txt", lt->tm_sec, lt->tm_min, lt->tm_hour, lt->tm_mday);
	fdToDisk = open(destino, O_CREAT | O_WRONLY, 0777);

	while ((cnt = read(srcfd, buf, sizeof(buf))) > 0) {
		if (len < 0)
			err_quit("helper.c:xfer_data:read");
		if ((len = write(fdToDisk, buf, cnt)) != cnt)
			err_quit("helper.c:xfer_data:write");
	}
}
