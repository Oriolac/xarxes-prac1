#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <stdarg.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "struct_f.h"

#define NUM_CHARS_ARXIU 15
#define NUM_CHARS_PRINT 50
#define LENGTH_BUFF 20

void actualitzar_hora(struct t *temps);
void error_arxiu_configuracio(struct t *temps, char arxiu[]);
void print_if_debug(int debug, struct t *temps, const char *fmt, ...);
void lectura_parametres(int argc, char** argv, struct t *temps, struct args *args);
FILE *obrir_arxius_config(char arxiu[NUM_CHARS_ARXIU], struct t *temps);
void configuracio_client(struct args *args, struct server *s, struct client *c);
void connexio_UDP(struct t *temps, struct args *args, struct server s, struct client c);

/*
 * Funció: main
 * -----------------
 * Funció principal de client.c
 * 		- Mira els paràmetres
 * 		- Connexió UDP
 */
int main(int argc, char **argv)
{
	/* Declaració de les variables */
	struct args args;
    struct t temps;
	struct server s;
	struct client c;

	print_if_debug(1, &temps, "Hola %s", "hols");

    args.debug = 0;

	/* Inicialitzar la hora per evitar el warning */
	temps.t=time(NULL);
	temps.tm=localtime(&temps.t);

	/* Lectura dels paràmetres i  opertura dels arxius */
	lectura_parametres(argc, argv, &temps, &args);

	configuracio_client(&args, &s, &c);

	/* Connexió UDP. */
	connexio_UDP(&temps, &args, s, c);


	exit(0);
}

/*	
 * Funció: actualitzar_hora
 * -----------------
 * Guarda el temps actual en H:M:S a la String hora.
 */
void actualitzar_hora(struct t *temps)
{
	temps->tm=localtime(&temps->t);
	strftime(temps->hora, 15, "%H:%M:%S", temps->tm);
}

/*
 * Funció: error_arxiu_configuracio
 * -----------------
 * Imprimeix per pantalla que no s'ha pogut obrir l'arxiu de configuració i finalitza el programa.
 */
void error_arxiu_configuracio(struct t *temps, char arxiu[])
{
	actualitzar_hora(temps);
	printf("%s: ERROR =>  No es pot obrir l'arxiu de configuració: %s\n", temps->hora, arxiu);
	exit(-1);
}

/*
*/
void print_if_debug(int debug, struct t *temps, const char *fmt, ...)
{
	char *string;
	va_list	args;
	va_start(args,fmt);
	if(debug == 1)
	{
		actualitzar_hora(temps);
		printf("%s: DEBUG => ",temps->hora);
		while( *fmt != '\0')
		{
			if(*fmt == '%' && *(fmt+1) == 's')
			{
				++fmt;
				string = va_arg(args,char*);
				printf("%s", string);
			} else {
				printf("%c", *fmt);
			}
			++fmt;
		}
		printf("\n");
	}
	
}

/*
 * Funció: lectura_parametres
 * -----------------
 * Llegeix tots els paràmetres passats a l'hora d'executar el client i decidex quins arxius
 * s'han d'obrir i si hi ha mode debug.
 */
void lectura_parametres(int argc, char** argv, struct t *temps, struct args *args)
{
    int i;
    char arxiuSoft[NUM_CHARS_ARXIU];
    char arxiuEquip[NUM_CHARS_ARXIU];

	/* Per defecte, l'arxiu de dades de configuració del client serà "client.cfg". No obrim el fitxer fins després 
	* ja que es poden produir canvis.
	* L'arxiu de configuració de l'equip per defecte és "boot.cfg."
	*/
	sprintf(arxiuSoft, "client.cfg");
	sprintf(arxiuEquip, "boot.cfg");

	/* Mira si existeixen paràmetres com el -d o -c */
	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-d") == 0){

			/* S'activa el Mode DEBUG */
			args->debug = 1;
			actualitzar_hora(temps);
			printf("%s: DEBUG =>  Mode DEBUG ON.\n", temps->hora);
		} else if(strcmp(argv[i], "-c") == 0){

			/* Es canvia el nom de l'arxiu. */
			if(argc <= i+1){
				error_arxiu_configuracio(temps, "");
			}
			sprintf(arxiuSoft, "%s",argv[i+1]);
			if(args->debug == 1){
				actualitzar_hora(temps);
				printf("%s: DEBUG =>  Arxiu de dades de software modificat: %s\n", temps->hora, arxiuSoft);
			}
		} else if(strcmp(argv[i], "-f") == 0){
			if(argc <= i+1){ 	
				error_arxiu_configuracio(temps, "");
			}
			sprintf(arxiuEquip, "%s", argv[i + 1]);
			if(args->debug == 1){
				actualitzar_hora(temps);
				printf("%s: DEBUG =>  Arxiu de configuració de l'equip modificat: %s\n", temps->hora, arxiuEquip);
			}
		}
	}

	print_if_debug(args->debug,temps, "Llegits paràmetres línia de comandes.");

	args->fitxerSoft = obrir_arxius_config(arxiuSoft, temps);
	print_if_debug(args->debug, temps, "Obert arxiu de configuració de software.");
	args->fitxerEquip = obrir_arxius_config(arxiuEquip, temps);
	print_if_debug(args->debug, temps, "Obert arxiu de configuració de l'equip.");
}

/* Intentem obrir l'arxiu de configuració i mirem si existeix. 
 */
FILE *obrir_arxius_config(char arxiu[NUM_CHARS_ARXIU], struct t *temps)
{
	FILE *fd;
	fd = fopen(arxiu, "rt");
	if(fd == NULL)
	{
		error_arxiu_configuracio(temps, arxiu);
	}
	return fd;
}
/*
 * Funció: configuracio_client
 * ----------------- 
 * Retorna una estructura tipus client.
 */
void configuracio_client(struct args *args, struct server *s, struct client *c)
{
	char buf[20];
	char buf2[20];

	printf("config_client reached\n");
	while(fscanf(args->fitxerSoft,"%s %s", buf, buf2) != EOF){
		if(strcmp(buf, "Nom") == 0){
			printf("Putting name in struct %s\n", buf2);
			strcpy(c->equip,buf2);
		} else if(strcmp(buf, "MAC") == 0){
			printf("Putting MAC in struct %s\n", buf2);
			strcpy(c->mac,buf2);
		} else if(strcmp(buf, "Server") == 0){
			printf("Putting Server in struct %s\n", buf2);
			strcpy(s->server,buf2);
		} else if(strcmp(buf, "Server-port") == 0){
			printf("Putting port in struct %s\n", buf2);
			s->serverPort=atoi(buf2);
		}
	}
}

/*
 * Funció: connexio_UDP
 * -----------------
 * Intenta obrir tots els arxius. En cas que no existeixin ho ha de notificar i marcar error.
 * En cas que existeixin i el mode DEBUG estigui activat, s'ha de notificar que s'ha obert. 
 */
void connexio_UDP(struct t *temps, struct args *args, struct server s, struct client c)
{
	struct sockaddr_in addr_serv;
	struct paquet_udp p;
	int fd;
	struct hostent *ent;
	int a;
	char dades[50];
	printf("Initialized variables to connect server udp\n");
	
	fd = socket(AF_INET,SOCK_DGRAM, 0);
	if(fd < 0)
	{
		actualitzar_hora(temps);
		printf("%s: ERROR =>  No s'ha pogut crear socket.\n", temps->hora);
		exit(-1);
	}
	printf("Reached get host by name\n");
	ent=gethostbyname(s.server);
	printf("later get host by name\n");
	if(!ent)
	{
		actualitzar_hora(temps);
		printf("%s: ERROR => No s'ha pogut trobar el host del servidor.\n", temps->hora);
		exit(-1);
	}

	printf("memset reached\n");
	memset(&addr_serv, 0, sizeof(struct sockaddr_in));
	addr_serv.sin_family=AF_INET;
	addr_serv.sin_addr.s_addr=((struct in_addr *) ent->h_addr_list[0])->s_addr;
	addr_serv.sin_port=htons(s.serverPort);
	printf("Adrr init\n");
	/* Paquet */

	memset(&p, 0, sizeof(p));
	p.type=1;
	strcpy(p.equip, c.equip);
	strcpy(p.mac, c.mac);
	strcpy(p.random_number, "123456");
	memset(&dades, 0, sizeof(dades));
	strcpy(p.dades, dades);
	

	a=sendto(fd,&p,sizeof(p),0,(struct sockaddr*)&addr_serv,sizeof(addr_serv));
    if(a<0)
    {
		actualitzar_hora(temps);
		printf("%s: ERROR => No s'ha pogut enviar paquet.\n", temps->hora);
		exit(-1);
	}
}