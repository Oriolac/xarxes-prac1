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
struct sockaddr_in addr_servidor(struct server s);
void recorregut_udp(int debug, int fd, struct paquet_udp p, struct sockaddr_in addr_serv, struct client c, struct server s);
void peticio_registre(int debug, int fd, struct paquet_udp p, struct sockaddr_in addr_serv, int t, int *nack, struct client c, struct server s);
struct paquet_udp read_feedback(int debug, int fd, int t);
void sendto_udp(int fd, struct paquet_udp paquet,struct sockaddr_in addr_serv);
void comunicacio_periodica(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, struct client c, struct server s);
int comprovacio_alive_ack(int debug,struct paquet_udp paquet1,struct paquet_udp paquet2, int count);
int es_servidor_correcte(struct paquet_udp paquet_recv, struct info_serv info_s);
int control_stop(int count_no_alive_ack);
struct paquet_udp escriure_paquet(int type ,struct client c, char * random);

#endif