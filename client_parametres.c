#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "client_funcions.h"

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
    char arxiu_soft[NUM_CHARS_ARXIU];
    char arxiu_equip[NUM_CHARS_ARXIU];

	/* Guardem tot allò per defecte. */
	sprintf(arxiu_soft, "client.cfg");
	sprintf(arxiu_equip, "boot.cfg");
    args->debug = 0;

	/* Mira si existeixen paràmetres com el -d o -c */
	for(i = 1; i < argc; i++){
		if(!strcmp(argv[i], "-d")){

			/* S'activa el Mode DEBUG */
			args->debug = 1;
			print_with_time("DEBUG => Mode DEBUG ON.");
		} else if(!strcmp(argv[i], "-c")){

			/* Es canvia el nom de l'arxiu. */
			canvi_nom_arxiu(argc,argv, i, arxiu_soft);
			print_if_debug(args->debug, "Arxiu de dades de software modificat: %s", arxiu_soft);
		} else if(!strcmp(argv[i], "-f")){
			canvi_nom_arxiu(argc, argv, i, arxiu_equip);
			print_if_debug(args->debug,"Arxiu de configuració de l'equip modificat: %s", arxiu_equip);
		}
	}
	print_if_debug(args->debug, "Llegits paràmetres línia de comandes.");

	args->fitxerSoft = obrir_arxius_config(arxiu_soft);
	print_if_debug(args->debug, "Obert arxiu de configuració de software.");
	args->fitxerEquip = obrir_arxius_config(arxiu_equip);
	print_if_debug(args->debug, "Obert arxiu de configuració de l'equip.");
}

/* 
 * Funció: obrir_arxius_config
 * -----------------
 * Intentem obrir l'arxiu de configuració i mirem si existeix. 
 */
FILE *obrir_arxius_config(char arxiu[])
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
		if(!strcmp(buf, "Nom")){
			strcpy(c->equip,buf2);
		} else if(!strcmp(buf, "MAC")){
			strcpy(c->mac,buf2);
		} else if(!strcmp(buf, "Server")){
			strcpy(s->server,buf2);
		} else if(!strcmp(buf, "Server-port")){
			s->serverPort=atoi(buf2);
		}
	}
	print_if_debug(args->debug, "El client és:\n\t\tNom: %s\n\t\tMAC: %s", c->equip, c->mac);
	print_if_debug(args->debug,"El servidor té:\n\t\tAdreça: %s\n\t\tPort: %i", s->server, s->serverPort); 
}