
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

char* tipus_pdu(unsigned char c)
{
	switch (c)
	{
		case 0:
			return "REGISTER_REQ";
		case 1:
			return "REGISTER_ACK";
		case 2:
			return "REGISTER_NACK";
		case 3:
			return "REGISTER_REJ";
		case 9:
			return "ERROR";
		case 10:
			return "ALIVE_INF";
		case 11:
			return "ALIVE_ACK";
		case 12:
			return "ALIVE_NACK";
		case 13:
			return "ALIVE_REJ";
		default:
			return "?";
	}
}