#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "struct_f.h"

#define NUM_CHARS 15
#define LENGTH_BUFF 20

void actualitzarHora(struct t *temps);
void errorArxiuConfiguracio(struct t *temps, char arxiu[]);
void lecturaParametres(int argc, char** argv, struct t *temps, struct args *args);
void obrirArxius(struct t *temps, struct args *args, char arxiuSoft[], char arxiuEquip[]);
void connexioUDP(struct t *temps, struct args *args, struct client c);
struct client configuracioClient(struct args *args);

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
	struct client c;

    args.debug = 0;

	/* Inicialitzar la hora per evitar el warning */
	temps.t=time(NULL);
	temps.tm=localtime(&temps.t);

	/* Lectura dels paràmetres i  opertura dels arxius */
	lecturaParametres(argc, argv, &temps, &args);

	c = configuracioClient(&args);

	/* Connexió UDP. */
	connexioUDP(&temps, &args, c);


	exit(0);
}

/*	
 * Funció: actualitzarHora
 * -----------------
 * Guarda el temps actual en H:M:S a la String hora.
 */
void actualitzarHora(struct t *temps)
{
	temps->tm=localtime(&temps->t);
	strftime(temps->hora, 15, "%H:%M:%S", temps->tm);
}

/*
 * Funció: errorArxiuConfiguracio
 * -----------------
 * Imprimeix per pantalla que no s'ha pogut obrir l'arxiu de configuració i finalitza el programa.
 */
void errorArxiuConfiguracio(struct t *temps, char arxiu[])
{
	actualitzarHora(temps);
	printf("%s: ERROR =>  No es pot obrir l'arxiu de configuració: %s\n", temps->hora, arxiu);
	exit(-1);
}

/*
 * Funció: lecturaParametres
 * -----------------
 * Llegeix tots els paràmetres passats a l'hora d'executar el client i decidex quins arxius
 * s'han d'obrir i si hi ha mode debug.
 */
void lecturaParametres(int argc, char** argv, struct t *temps, struct args *args)
{
    int i;
    char arxiuSoft[NUM_CHARS];
    char arxiuEquip[NUM_CHARS];

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
			actualitzarHora(temps);
			printf("%s: DEBUG =>  Mode DEBUG ON.\n", temps->hora);
		} else if(strcmp(argv[i], "-c") == 0){

			/* Es canvia el nom de l'arxiu. */
			if(argc <= i+1){
				errorArxiuConfiguracio(temps, "");
			}
			sprintf(arxiuSoft, "%s",argv[i+1]);
			if(args->debug == 1){
				actualitzarHora(temps);
				printf("%s: DEBUG =>  Arxiu de dades de software modificat: %s\n", temps->hora, arxiuSoft);
			}
		} else if(strcmp(argv[i], "-f") == 0){
			if(argc <= i+1){ 	
				errorArxiuConfiguracio(temps, "");
			}
			sprintf(arxiuEquip, "%s", argv[i + 1]);
			if(args->debug == 1){
				actualitzarHora(temps);
				printf("%s: DEBUG =>  Arxiu de configuració de l'equip modificat: %s\n", temps->hora, arxiuEquip);
			}
		}
	}
	if(args->debug == 1){
		actualitzarHora(temps);
		printf("%s: DEBUG =>  Llegits paràmetres línia de comandes.\n", temps->hora);
	}
	obrirArxius(temps, args, arxiuSoft, arxiuEquip);
}
/*
 * Funció: obrirArxius
 * -----------------
 * Intenta obrir tots els arxius. En cas que no existeixin ho ha de notificar i marcar error.
 * En cas que existeixin i el mode DEBUG estigui activat, s'ha de notificar que s'ha obert. 
 */
void obrirArxius(struct t *temps, struct args *args, char arxiuSoft[], char arxiuEquip[])
{
	/* Intentem obrir l'arxiu de configuració i mirem si existeix. */
	args->fitxerSoft = fopen(arxiuSoft, "rt");
	if(args->fitxerSoft == NULL){
		errorArxiuConfiguracio(temps, arxiuSoft);
	} else if(args->debug == 1){
		actualitzarHora(temps);
		printf("%s: DEBUG =>  Obert arxiu de configuració de software.\n", temps->hora);
	}

	/* Intentem obrir l'arxiu de configuració i mirem si existeix. */
	args->fitxerEquip = fopen(arxiuEquip, "r");
	if(args->fitxerEquip == NULL){
		errorArxiuConfiguracio(temps, arxiuEquip);
	} else if(args->debug == 1){
		actualitzarHora(temps);
		printf("%s: DEBUG =>  Obert arxiu de configuració de l'equip.\n", temps->hora);
	}

}

/*
 * Funció: configuracioClient
 * ----------------- 
 * Retorna una estructura tipus client.
 */
struct client configuracioClient(struct args *args)
{
	struct client c;
	char buf[100];
	char buf2[100];
	while(fscanf(args->fitxerSoft,"%s %s", buf, buf2) != EOF){
		if(strcmp(buf, "Nom") == 0){
			strcpy(c.nom,buf2);
		} else if(strcmp(buf, "MAC") == 0){
			strcpy(c.mac,buf2);
		} else if(strcmp(buf, "Server") == 0){
			strcpy(c.server,buf2);
		} else if(strcmp(buf, "Server-port") == 0){
			c.serverPort=atoi(buf2);
		}
	}
	return c;
}

/*
 * Funció: connexioUDP
 * -----------------
 * Intenta obrir tots els arxius. En cas que no existeixin ho ha de notificar i marcar error.
 * En cas que existeixin i el mode DEBUG estigui activat, s'ha de notificar que s'ha obert. 
 */
void connexioUDP(struct t *temps, struct args *args, struct client c)
{
	struct sockaddr_in addr_serv;
	int fd;
	struct hostent *ent;
	int a;
	char paquet[LENGTH_BUFF];

	
	fd = socket(AF_INET,SOCK_DGRAM, 0);
	if(fd < 0)
	{
		actualitzarHora(temps);
		printf("%s: ERROR =>  No s'ha pogut crear socket.\n", temps->hora);
		exit(-1);
	}
	ent=gethostbyname(c.server);
	if(!ent)
	{
		actualitzarHora(temps);
		printf("%s: ERROR => No s'ha pogut trobar el host del servidor.\n", temps->hora);
		exit(-1);
	}

	memset(&addr_serv, 0, sizeof(struct sockaddr_in));
	addr_serv.sin_family=AF_INET;
	addr_serv.sin_addr.s_addr=((struct in_addr *) ent->h_addr_list[0])->s_addr;
	addr_serv.sin_port=htons(c.serverPort);

	/* Paquet */
	
	a=sendto(fd,temps->hora,strlen(temps->hora)+1,0,(struct sockaddr*)&addr_serv,sizeof(addr_serv));
    if(a<0)
    {
		actualitzarHora(temps);
		printf("%s: ERROR => No s'ha pogut enviar paquet.\n", temps->hora);
		exit(-1);
	}
}