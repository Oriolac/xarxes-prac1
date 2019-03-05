#include <stdarg.h>
#include "struct_f.h"

#define NUM_CHARS_ARXIU 15
#define NUM_CHARS_PRINT 50
#define LENGTH_BUFF 20

#ifndef FUNCIONS
#define FUNCIONS

void actualitzar_hora(struct t *temps);
void print_with_time(const char *fmt, ...);
void print_string(const char *fmt, va_list args);

void error_arxiu_configuracio( char arxiu[]);
void print_if_debug(int debug, const char *fmt, ...);

void canvi_nom_arxiu(int argc, char **argv, int i, char *arxiu);
void lectura_parametres(int argc, char** argv, struct args *args);
FILE *obrir_arxius_config(char arxiu[]);
void configuracio_software(struct args *args, struct server *s, struct client *c);

void connexio_UDP(int debug, struct server s, struct client c);


#endif