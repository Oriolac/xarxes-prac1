#include <stdarg.h>
#include "struct_f.h"
#include <netinet/in.h>

#define NUM_CHARS_ARXIU 15
#define NUM_CHARS_PRINT 50
#define LENGTH_BUFF 20

#ifndef FUNCIONS
#define FUNCIONS

/* client_main.c        */
char* tipus_pdu(unsigned char c);

/* client_prints.c      */
void actualitzar_hora(struct t *temps);
void print_with_time(const char *fmt, ...);
void print_string(const char *fmt, va_list args);
void error_arxiu_configuracio( char arxiu[]);
void print_if_debug(int debug, const char *fmt, ...);

/* client_parametres.c  */
void canvi_nom_arxiu(int argc, char **argv, int i, char *arxiu);
void lectura_parametres(int argc, char** argv, struct args *args);
FILE *obrir_arxius_config(char arxiu[]);
void configuracio_software(struct args *args, struct server *s, struct client *c);

/* client_udp.c         */
void connexio_UDP(int debug, struct server s, struct client c);
void recorregut_udp(int debug, int fd, struct paquet_udp p, struct sockaddr_in addr_serv);
void socket_udp(int debug, int fd, struct paquet_udp p, struct sockaddr_in addr_serv, int t, int *nack);
struct paquet_udp read_feedback(int debug, int fd, int t);
void sendto_udp(int fd, struct paquet_udp paquet,struct sockaddr_in addr_serv);
void comunicacio_periodica(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv);
int comprovacio_alive_ack(int debug,struct paquet_udp paquet);

#endif