
#include "client_funcions.h"


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
