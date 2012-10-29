#ifndef HELPER_H_
#define HELPER_H_
void err_quit(char *msg);
void xfer_data(int srcfd, int tgtfd);
void herr_quit(char *msg);
void tranferir(int srcfd);
void *copiarSockets(void *arrayDsc);
#endif /* HELPER_H_ */

