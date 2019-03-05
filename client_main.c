#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <stdarg.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "client_funcions.h"

#define NUM_CHARS_ARXIU 15
#define NUM_CHARS_PRINT 50
#define LENGTH_BUFF 20

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
	struct server s;
	struct client c;


	/* Lectura dels paràmetres i  opertura dels arxius */
	lectura_parametres(argc, argv, &args);
	configuracio_software(&args, &s, &c);

	/* Connexió UDP. */
	connexio_UDP(args.debug, s, c);


	exit(0);
}


void canvi_nom_arxiu(int argc, char **argv, int i, char *arxiu)
{
	if(argc <= i+1)
	{
		error_arxiu_configuracio("");
	}
	sprintf(arxiu, "%s", argv[i+1]);
}

/*
 * Funció: lectura_parametres
 * -----------------
 * Llegeix tots els paràmetres passats a l'hora d'executar el client i decidex quins arxius
 * s'han d'obrir i si hi ha mode debug.
 */
void lectura_parametres(int argc, char** argv, struct args *args)
{
    int i;
    char arxiuSoft[NUM_CHARS_ARXIU];
    char arxiuEquip[NUM_CHARS_ARXIU];

	/* Guardem tot allò per defecte. */
	sprintf(arxiuSoft, "client.cfg");
	sprintf(arxiuEquip, "boot.cfg");
    args->debug = 0;

	/* Mira si existeixen paràmetres com el -d o -c */
	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-d") == 0){

			/* S'activa el Mode DEBUG */
			args->debug = 1;
			print_with_time("DEBUG => Mode DEBUG ON.");
		} else if(strcmp(argv[i], "-c") == 0){

			/* Es canvia el nom de l'arxiu. */
			canvi_nom_arxiu(argc,argv, i, arxiuSoft);
			print_if_debug(args->debug, "Arxiu de dades de software modificat: %s", arxiuSoft);
		} else if(strcmp(argv[i], "-f") == 0){
			canvi_nom_arxiu(argc, argv, i, arxiuEquip);
			print_if_debug(args->debug,"Arxiu de configuració de l'equip modificat: %s", arxiuEquip);
		}
	}
	print_if_debug(args->debug, "Llegits paràmetres línia de comandes.");

	args->fitxerSoft = obrir_arxius_config(arxiuSoft);
	print_if_debug(args->debug, "Obert arxiu de configuració de software.");
	args->fitxerEquip = obrir_arxius_config(arxiuEquip);
	print_if_debug(args->debug, "Obert arxiu de configuració de l'equip.");
}

/* 
 * Funció: obrir_arxius_config
 * -----------------
 * Intentem obrir l'arxiu de configuració i mirem si existeix. 
 */
FILE *obrir_arxius_config(char arxiu[NUM_CHARS_ARXIU])
{
	FILE *fd;
	fd = fopen(arxiu, "rt");
	if(fd == NULL)
	{
		error_arxiu_configuracio(arxiu);
	}
	return fd;
}
/*
 * Funció: configuracio_software
 *
 * ----------------- 
 * Mira el fitxer de configuració de software.
 */
void configuracio_software(struct args* args, struct server *s, struct client *c)
{
	char buf[20];
	char buf2[20];

	while(fscanf(args->fitxerSoft,"%s %s", buf, buf2) != EOF){
		if(strcmp(buf, "Nom") == 0){
			strcpy(c->equip,buf2);
		} else if(strcmp(buf, "MAC") == 0){
			strcpy(c->mac,buf2);
		} else if(strcmp(buf, "Server") == 0){
			strcpy(s->server,buf2);
		} else if(strcmp(buf, "Server-port") == 0){
			s->serverPort=atoi(buf2);
		}
	}
	print_if_debug(args->debug, "El client és:\n\t\tNom: %s\n\t\tMAC: %s", c->equip, c->mac);
	print_if_debug(args->debug,"El servidor té:\n\t\tAdreça: %s\n\t\tPort: %i", s->server, s->serverPort); 
}

/*
 * Funció: connexio_UDP
 * -----------------
 * Intenta obrir tots els arxius. En cas que no existeixin ho ha de notificar i marcar error.
 * En cas que existeixin i el mode DEBUG estigui activat, s'ha de notificar que s'ha obert. 
 */
void connexio_UDP(int debug, struct server s, struct client c)
{
	struct sockaddr_in addr_serv;
	struct paquet_udp p;
	int fd;
	struct hostent *ent;
	int a;
	char dades[50];
	
	fd = socket(AF_INET,SOCK_DGRAM, 0);
	if(fd < 0)
	{
		print_with_time("ERROR =>  No s'ha pogut crear socket.");
		exit(-1);
	}
	print_if_debug(debug, "Realitzat el socket UDP.");

	ent=gethostbyname(s.server);
	if(!ent)
	{
		print_with_time("ERROR => No s'ha pogut trobar el host del servidor.");
		exit(-1);
	}

	memset(&addr_serv, 0, sizeof(struct sockaddr_in));
	addr_serv.sin_family=AF_INET;
	addr_serv.sin_addr.s_addr=((struct in_addr *) ent->h_addr_list[0])->s_addr;
	addr_serv.sin_port=htons(s.serverPort);

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
		print_with_time("ERROR => No s'ha pogut enviar paquet.");
		exit(-1);
	}
}