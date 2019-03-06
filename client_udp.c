#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/select.h>
#include <signal.h>

#include "client_funcions.h"

#define N 3
#define T 2
#define M 4
#define P 8
#define S 5
#define Q 3

/*
 * Funció: connexio_UDP
 * -----------------
 * Intenta obrir tots els arxius. En cas que no existeixin ho ha de notificar i marcar error.
 * En cas que existeixin i el mode DEBUG estigui activat, s'ha de notificar que s'ha obert. 
 */
void connexio_UDP(int debug, struct server s, struct client c)
{
	struct sockaddr_in addr_serv;
	struct paquet_udp paquet;
	int fd;
	struct hostent *ent;
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
    print_if_debug(debug, "Emplenada l'adreça del servidor i el seu port.");

	/* Paquet */
	memset(&paquet, 0, sizeof(paquet));
	paquet.type=0;
	strcpy(paquet.equip, c.equip);
	strcpy(paquet.mac, c.mac);
	strcpy(paquet.random_number, "000000");
	memset(&dades, 0, sizeof(dades));
	strcpy(paquet.dades, dades);
    print_if_debug(debug, "Emplenat el paquet UDP del client.");
	
    recorregut_udp(debug, fd, paquet, addr_serv);
}

void recorregut_udp(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv)
{
	int n;
	int t;
	int p;
	int m = M;
	int s = S;
	int numIntents = 1;
	int q = Q;

	print_if_debug(debug, "Inici bucle de servei equip: %s", paquet.equip);
	print_with_time("MSG. => Equip passa a l'estat DISCONNECTED");
	while(q > 0)
	{
		n = N;
		t = T;
		p = P;
		while(n > 0)
		{
			print_if_debug(debug, "Registre equip. Intent: %i", numIntents);
			socket_udp(debug, fd, paquet, addr_serv, t);
			n--;
			numIntents++;
		}
		while( m*T > t)
		{
			t += T;
			print_if_debug(debug, "Registre equip. Intent %i.", numIntents, t);
			socket_udp(debug, fd, paquet, addr_serv, t);
			numIntents++;
		}
		t = m * T;
		while(p > 0)
		{
			print_if_debug(debug, "Registre equip. Intent %i", numIntents);
			socket_udp(debug, fd, paquet, addr_serv, t);
			p--;
			numIntents++;
		}
		print_if_debug(debug,"S'espera %i segons fins nou procés de registre", s);
		sleep(s);
		q--;
	}
	print_if_debug(debug,"No s'ha rebut acceptació. Es seguiran enviant paquets però incrementant els segons.");
}

void socket_udp(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, int t)
{
	int a;
	
	a=sendto(fd,&paquet,sizeof(paquet),0,(struct sockaddr*)&addr_serv,sizeof(addr_serv));
    if(a<0)
    {
		print_with_time("ERROR => No s'ha pogut enviar paquet.");
		exit(-1);
	}

	print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
	print_with_time("MSG. => Client passa a l'estat: WAIT_REG");
	read_feedback_register(debug, fd, paquet, t);

}

void read_feedback_register(int debug, int fd, struct paquet_udp paquet, int t)
{
	int a;
	fd_set readfds;

	struct timeval timev;
	timev.tv_sec = t;
	timev.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	a = select(fd+1, &readfds, NULL,NULL, &timev);
	if( a < 0 )
	{
		print_with_time("ERROR => select.\n");
	} else if(FD_ISSET(fd, &readfds))
	{
		memset(&paquet, 0, sizeof(paquet));
		a=recvfrom(fd, &paquet, sizeof(paquet),0, (struct sockaddr * )0, (socklen_t *)0);
		if( a < 0)
		{
			print_with_time("ERROR => No s'ha rebut el socket.");
			exit(-1);
		}
		print_if_debug(debug,"S'ha rebut el paquet: tipus=%i, nom=%s, mac=%s, alea%s, dades=%s", tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
		sleep(t);
	}
}
