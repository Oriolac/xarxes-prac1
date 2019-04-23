
#include "client_funcions.h"
#include <sys/types.h>

int main(int argc, char **argv)
{
	struct args args;
	struct server s;
	struct client c;

	lectura_parametres(argc, argv, &args);
	configuracio_software(&args, &s, &c);
	connexio_UDP(args.debug, s, c, args.fitxer_equip);

	exit(0);
}

char* tipus_pdu(unsigned char c)
{
	switch (c)
	{
		case 0x00:
			return "REGISTER_REQ";
		case 0x01:
			return "REGISTER_ACK";
		case 0x02:
			return "REGISTER_NACK";
		case 0x03:
			return "REGISTER_REJ";
		case 0x09:
			return "ERROR";
		case 0x10:
			return "ALIVE_INF";
		case 0x11:
			return "ALIVE_ACK";
		case 0x12:
			return "ALIVE_NACK";
		case 0x13:
			return "ALIVE_REJ";
		case 0x20:
			return "SEND_FILE";
		case 0x21:
			return "SEND_ACK";
		case 0x22:
			return "SEND_NACK";
		case 0x23:
			return "SEND_REJ";
		case 0x24:
			return "SEND_DATA";
		case 0x25:
			return "SEND_END";
		default:
			return "?";
	}
}