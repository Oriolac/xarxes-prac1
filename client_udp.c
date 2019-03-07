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
#define R 3
#define U 3

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

	paquet = escriure_paquet(debug,"000000",c,0);
	
    recorregut_udp(debug, fd, paquet, addr_serv, c);
}

void recorregut_udp(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, struct client c)
{
	struct temporitzadors tors;
	int nack = 0;
	tors.numIntents = 1;
	tors.q = Q;

	print_if_debug(debug, "Inici bucle de servei equip: %s", paquet.equip);
	print_with_time("MSG.  => Equip passa a l'estat DISCONNECTED");

	while(tors.q > 0)
	{
		tors.n = N;
		tors.t = T;
		tors.p = P;
		nack = 0;
		while(tors.n > 0)
		{
			print_if_debug(debug, "Registre equip. Intent: %i", tors.numIntents);
			socket_udp(debug, fd, paquet, addr_serv, tors.t, &nack, c);
			tors.numIntents++;
			tors.n--;
		}
		while(!nack && M*T > tors.t)
		{
			tors.t += T;
			print_if_debug(debug, "Registre equip. Intent %i.", tors.numIntents);
			socket_udp(debug, fd, paquet, addr_serv, tors.t, &nack,c);
			tors.numIntents++;
		}
		tors.t = M * T;
		while(!nack && tors.p > 0)
		{
			print_if_debug(debug, "Registre equip. Intent %i", tors.numIntents);
			socket_udp(debug, fd, paquet, addr_serv, tors.t, &nack,c);
			tors.numIntents++;
			tors.p--;
		}
		print_if_debug(debug,"S'espera %i segons fins nou procés de registre", S);
		sleep(S);
		tors.q--;
	}
	print_if_debug(debug,"No s'ha rebut acceptació. Es seguiran enviant paquets però incrementant els segons.");
}

void socket_udp(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, int t, int *nack, struct client c)
{
	sendto_udp(fd, paquet, addr_serv);

	print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
	print_with_time("MSG.  => Client passa a l'estat: WAIT_REG");

	paquet = read_feedback(debug, fd, t);

	switch (paquet.type)
	{
		case 3:
			print_with_time("MSG.  => El registre ha estat rebutjat. Motiu: Rebut paquet REGISTER_REJ");
			print_with_time("MSG.  => Equip passa a l'estat DISCONNECTED");
			exit(0);
		case 2:
			*nack = 1;
			print_if_debug(debug, "Rebut paquet REGISTER_NACK");
			break;
		case 1:
			print_if_debug(debug, "Rebut paquet REGISTER_ACK");
			print_with_time("MSG.  => Equip passa a l'estat REGISTERED");
			print_with_time("INFO  => Acceptada subscripció amb servidor: (nom:%s, mac:%s, alea:%s, port tcp:%s)",paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
			comunicacio_periodica(debug, fd,paquet,addr_serv,c);
			break;
		default:
			break;
	}

}

void sendto_udp(int fd, struct paquet_udp paquet,struct sockaddr_in addr_serv)
{
	int a;
	a=sendto(fd,&paquet,sizeof(paquet),0,(struct sockaddr*)&addr_serv,sizeof(addr_serv));
    if(a<0)
    {
		print_with_time("ERROR => No s'ha pogut enviar paquet.");
		exit(-1);
	}

}

struct paquet_udp read_feedback(int debug, int fd, int t)
{
	int a;
	fd_set readfds;
	struct paquet_udp paquet;
	struct timeval timev;

	memset(&paquet, 0, sizeof(paquet));

	timev.tv_sec = t;
	timev.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	a = select(fd+1, &readfds, NULL,NULL, &timev);
	if( a < 0 )
	{
		print_with_time("ERROR => select.\n");
		paquet.type =4;
		return paquet;
	} else if(FD_ISSET(fd, &readfds))
	{
		a=recvfrom(fd, &paquet, sizeof(paquet),0, (struct sockaddr * )0, (socklen_t *)0);
		if( a < 0)
		{
			print_with_time("ERROR => No s'ha rebut el socket.");
			exit(-1);
		}
		print_if_debug(debug,"S'ha rebut el paquet: tipus=%s, nom=%s, mac=%s, alea=%s, dades=%s", tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
		sleep(t);
		return paquet;
	} 
	return paquet;
}

void comunicacio_periodica(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, struct client c)
{
	int stop;
	int primer_alive;
	int count_no_alive_ack;
	struct paquet_udp paquet_sendto;
	struct paquet_udp paquet_recv;

	print_if_debug(debug, "Començant la comunicació periòdica.");

	stop = 0;
	count_no_alive_ack = 0;
	primer_alive = 0;

	while(!stop)
	{
		paquet_sendto = escriure_paquet(debug, paquet.random_number, c,16);
		sendto_udp(fd, paquet_sendto, addr_serv);
		print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet_sendto),tipus_pdu(paquet_sendto.type), paquet_sendto.equip, paquet_sendto.mac, paquet_sendto.random_number, paquet_sendto.dades);
		sleep(2);
		paquet_recv = read_feedback(debug, fd, R);
		switch (paquet_recv.type)
		{
			case 0:
				count_no_alive_ack++;
				stop=control_stop(count_no_alive_ack);
				break;
			case 17:
				count_no_alive_ack = comprovacio_alive_ack(debug, paquet_recv, paquet_sendto, count_no_alive_ack);
				stop=control_stop(count_no_alive_ack);
				if(primer_alive == 0)
				{
					print_with_time("MSG.  => L'equip passa a l'estat ALIVE.");
					primer_alive = 1;
				}
				break;
			case 19:
				print_with_time("MSG.  => Equip passa a l'estat DISCONNECTED");
				exit(-1);
				break;
			default:
				print_with_time("ERROR => Rebut paquet no disponible. Intentem tornar a fer el registre.");
				stop=1;
				break;
		}
	}
	print_if_debug(debug, "Marxem de la comunicació periòdica");
}

int control_stop(int count_no_alive_ack)
{
	if(U <= count_no_alive_ack)
	{
		print_with_time("ERROR => No s'han rebut %i ALIVE_ACK", U, count_no_alive_ack);
		return 1;
	}
	return 0;
}

int comprovacio_alive_ack(int debug,struct paquet_udp paquet1, struct paquet_udp paquet2, int count)
{
	if(strcmp(paquet1.random_number, paquet2.random_number) != 0)
	{
		return count+1;
	}
	return count;
}

struct paquet_udp escriure_paquet(int debug, char *random, struct client c, int i)
{
	struct paquet_udp p;
	memset(&p,0, sizeof(p));
	p.type= (unsigned char) i;
	strcpy(p.equip, c.equip);
	strcpy(p.mac, c.mac);
	strcpy(p.random_number, random);
	strcpy(p.dades, "");
	return p;
}