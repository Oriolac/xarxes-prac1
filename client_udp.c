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

#include <unistd.h>

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
 * Crea el socket, agafa l'adreça del servidor i comença les peticions de registre.
 */
void connexio_UDP(int debug, struct server s, struct client c, char boot_file[])
{
	struct sockaddr_in addr_serv;
	struct paquet_udp paquet;
	int fd;
	
	fd = socket(AF_INET,SOCK_DGRAM, 0);
	if(fd < 0)
	{
		print_with_time("ERROR =>  No s'ha pogut crear socket.");
		exit(-1);
	}
	print_if_debug(debug, "Realitzat el socket UDP.");

	addr_serv = addr_servidor(s);
    print_if_debug(debug, "Emplenada l'adreça del servidor i el seu port.");

	paquet = escriure_paquet(0, c,"000000");
	
    recorregut_udp(debug, fd, paquet, addr_serv, c, s, boot_file);
}

/*
 * Funció: addr_servidor
 * -----------------
 * Retorna l'adreça del servidor donada una estructura server s
 */
struct sockaddr_in addr_servidor(struct server s)
{
	struct hostent *ent;
	struct sockaddr_in addr_serv;
	ent=gethostbyname(s.server);
	if(!ent)
	{
		print_with_time("ERROR => No s'ha pogut trobar el host del servidor.");
		exit(-1);
	}

	memset(&addr_serv, 0, sizeof(struct sockaddr_in));
	addr_serv.sin_family=AF_INET;
	addr_serv.sin_addr.s_addr=((struct in_addr *) ent->h_addr_list[0])->s_addr;
	addr_serv.sin_port=htons(s.server_port);

	return addr_serv;
}

/*
 * Funció: recorregut_udp
 * -----------------
 * Realitza la temporització del registre
 */
void recorregut_udp(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, struct client c, struct server s, char boot_file[])
{
	struct temporitzadors tors;
	int nack = 0;
	char aleatori[7];
	int pid;
	int pipe_comandes[2];

	tors.numIntents = 1;
	tors.q = Q;


	if(pipe(pipe_comandes) == -1)
	{
		print_with_time("ERROR => No s'ha pogut crear el pipe per passar dades entre processos");
		exit(-1);
	}

	pid = fork();
	if(pid == -1)
	{
		print_with_time("ERROR => No s'ha pogut crear el procés fill per espera de comandes.");
		exit(-1);
	} else if(pid == 0)
	{
		close(0);
		close(pipe_comandes[1]);
	} else
	{
		close(pipe_comandes[0]);
		espera_comandes_consola(debug, pipe_comandes, pid);
	}


	print_if_debug(debug, "Inici bucle de servei equip: %s", paquet.equip);
	print_with_time("MSG.  => Equip passa a l'estat DISCONNECTED");

	while(tors.q > 0)
	{
		sprintf(aleatori, "000000");
		tors.n = N;
		tors.t = T;
		tors.p = P;
		nack = 0;
		while(!nack && tors.n > 0)
		{
			print_if_debug(debug, "Registre equip. Intent: %i", tors.numIntents);
			paquet = escriure_paquet(0x00, c, aleatori);
			peticio_registre(debug, fd, paquet, addr_serv, tors.t, &nack, c, s, aleatori, pipe_comandes, boot_file);
			tors.numIntents++;
			tors.n--;
			tors.p--;
		}
		while(!nack && M*T > tors.t)
		{
			tors.t += T;
			print_if_debug(debug, "Registre equip. Intent %i.", tors.numIntents);
			paquet = escriure_paquet(0x00, c, aleatori);
			peticio_registre(debug, fd, paquet, addr_serv, tors.t, &nack, c, s, aleatori, pipe_comandes, boot_file);
			tors.numIntents++;
			tors.p--;
		}
		tors.t = M * T;
		while(!nack && tors.p > 0)
		{
			print_if_debug(debug, "Registre equip. Intent %i", tors.numIntents);
			paquet = escriure_paquet(0x00, c, aleatori);
			peticio_registre(debug, fd, paquet, addr_serv, tors.t, &nack, c, s, aleatori, pipe_comandes, boot_file);
			tors.numIntents++;
			tors.p--;
		}
		if(tors.q > 0)
		{
		    print_if_debug(debug,"S'espera %i segons fins nou procés de registre", S);
		}
		sleep(S);
		tors.q--;
	}
	print_if_debug(debug,"No s'ha rebut acceptació. Nombre de intents: %i",  tors.numIntents);
}


/*
 * Funció: peticio_registre
 * -----------------
 * Envia el paquet de registre i mira si reb confirmació
 */
void peticio_registre(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, int t, int *nack, struct client c, struct server s, char aleatori[], int pipe_comandes[], char boot_file[])
{
	sendto_udp(fd, paquet, addr_serv);

	print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
	print_with_time("MSG.  => Client passa a l'estat: WAIT_REG");

	paquet = read_feedback(debug, fd, t);

	switch (paquet.type)
	{
		case 0x03:
			print_with_time("MSG.  => El registre ha estat rebutjat. Motiu: Rebut paquet REGISTER_REJ");
			print_with_time("MSG.  => Equip passa a l'estat DISCONNECTED");
			exit(0);
		case 0x02:
			*nack = 1;
			print_if_debug(debug, "Rebut paquet REGISTER_NACK");
			break;
		case 0x01:
			print_if_debug(debug, "Rebut paquet REGISTER_ACK");
			print_with_time("MSG.  => Equip passa a l'estat REGISTERED");
			print_with_time("INFO  => Acceptada subscripció amb servidor: (nom:%s, mac:%s, alea:%s, port tcp:%s)",paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
			comunicacio_periodica(debug, fd, paquet, addr_serv, c, s, pipe_comandes, nack, boot_file);
			strcpy(aleatori, paquet.random_number);
			break;
		default:
			break;
	}

}
/*
 * Funció: sendto_udp
 * -----------------
 * Realitza l'enviament del paquet del client
 */
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

/*
 * Funció: read_feedback
 * -----------------
 * Retorna el paquet del servidor, en cas que no hi hagi hagut comunicació, retorna estructura buida.
 */
struct paquet_udp read_feedback(int debug, int fd, int t)
{
	struct paquet_udp paquet;
	int a;
	fd_set readfds;
	struct timeval timev;

	memset(&paquet, 0, sizeof(paquet));
	paquet.type = 0x12;

	timev.tv_sec = t;
	timev.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	a = select(fd+1, &readfds, NULL,NULL, &timev);
	if( a < 0 )
	{
		print_with_time("ERROR => Select en rebre paquet.\n");
		paquet.type = 0x09;
		return paquet;
	} else if(FD_ISSET(fd, &readfds))
	{
		a=recvfrom(fd, &paquet, sizeof(paquet),0, (struct sockaddr * )0, (socklen_t *) 0);
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


/*
 * Funció: comunicacio_periodica
 * -----------------
 * Va enviant paquets ALIVE_INF i, segons el que rep, continua en el mateix estat o canvia.
 */
void comunicacio_periodica(int debug, int fd, struct paquet_udp paquet, struct sockaddr_in addr_serv, struct client c, struct server s, int pipe_comandes[], int *nack, char boot_file[])
{
	int stop;
	int primer_alive;
	int count_no_alive_ack;
	struct paquet_udp alive;
	struct paquet_udp paquet_recv;
	struct info_serv info_server;

	print_if_debug(debug, "Començant la comunicació periòdica.");
	memset(&info_server, 0, sizeof(info_server));
	strcpy(info_server.nom, paquet.equip);
	strcpy(info_server.ip, s.server);
	strcpy(info_server.mac, paquet.mac);
	strcpy(info_server.aleatori, paquet.random_number);
	info_server.port_tcp = atoi(paquet.dades);

	stop = 0;
	count_no_alive_ack = 0;
	primer_alive = 0;

	while(!stop)
	{
		alive = escriure_paquet(0x10, c, paquet.random_number);
		sendto_udp(fd, alive, addr_serv);
		print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(alive),tipus_pdu(alive.type), alive.equip, alive.mac, alive.random_number, alive.dades);
		paquet_recv = read_feedback(debug, fd, R);
		switch (paquet_recv.type)
		{
			case 0x12:
				/* ALIVE_NACK */
				count_no_alive_ack++;
				stop=control_stop(count_no_alive_ack);
				break;
			case 0x11:
				/* ALIVE_ACK */
				if(es_servidor_correcte(paquet_recv, info_server))
				{
					count_no_alive_ack = 0;
					if(!primer_alive)
					{
						print_with_time("MSG.  => L'equip passa a l'estat ALIVE.");
						primer_alive = 1;
					}
				} else {
					print_with_time("INFO  => Error recepció paquet UDP. Servidor incorrecte (correcte: nom=%s, ip=%s, mac=%s, alea=%s)", info_server.nom, info_server.ip, info_server.mac, info_server.aleatori);
					count_no_alive_ack++;
					stop = control_stop(count_no_alive_ack);
				}
				break;
			case 0x13:
				/* ALIVE_NACK */
				print_with_time("MSG.  => Equip passa a l'estat DISCONNECTED. Motiu: Suplantació identitat");
				*nack = 1;
				stop = 1;
				break;
		    case 0x01:
				/* REGISTER_ACK */
                print_if_debug(debug, "Rebut paquet REGISTER_ACK");
                print_with_time("MSG.  => Equip passa a l'estat REGISTERED");
                print_with_time("INFO  => Acceptada subscripció amb servidor: (nom:%s, mac:%s, alea:%s, port tcp:%s)",paquet_recv.equip, paquet_recv.mac, paquet_recv.random_number, paquet_recv.dades);
                strcpy(paquet.random_number, paquet_recv.random_number);
				primer_alive = 0;
				break;
			default:
				print_with_time("ERROR => Rebut paquet no disponible. Intentem tornar a fer el registre.");
				stop=1;
				break;
		}

		switch (comanda(debug, pipe_comandes))
		{
			case 1:
			    /* comanda quit */
				close(fd);
				exit(1);
				break;
		    case 2:
		        /* comanda send-conf */
		        send_conf_command(debug, s, info_server, c, paquet.random_number, boot_file);
		        break;
		    case 3:
		        /* comanda get-conf */
				get_conf_command(debug, s, info_server, c, paquet.random_number, boot_file);
		        break;
			default:
				break;
		}

	}
	print_if_debug(debug, "Marxem de la comunicació periòdica");
}

/*
 * Funció: espera_comandes_consola
 * -----------------
 * Envia la informació al procés fill quan es pugui del que es fiqui per pantalla
 */
void espera_comandes_consola(int debug, int pipe_comandes[2], int pid)
{
	char comanda[20];
	int quit;

	int a;
	int fd;
	fd_set writefds;
		
	print_if_debug(debug, "Creat el procés fill");
	print_if_debug(debug, "El procés pare començarà a llegir comandes per consola.");

	fd = pipe_comandes[1];

	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);

	quit = 0;
	while(!quit)
	{
		a = select(fd+1, NULL, &writefds, NULL, NULL);
		if( a < 0 )
		{
			print_with_time("ERROR => Select que espera que es pugui enviar informació al procés fill.\n");
		} else if(FD_ISSET(fd, &writefds)){
			memset(comanda,'\0',sizeof(comanda));
			read(0, comanda, sizeof(comanda));
			sprintf(comanda, "%s", comanda);
			write(pipe_comandes[1], comanda, sizeof(comanda));
			if(strcmp("quit\n", comanda) == 0)
			{
				quit = 1;
				sleep(1);
			}
		}
	}
	exit(1);
}

/*
 * Funció: comanda
 * -----------------
 * Retorna un nombre enter segons la comanda.
 */
int comanda(int debug, int pipe_comandes[2]){
	char comanda[20];
	int a;
	int fd;
	fd_set readfds;
	struct timeval timev;

	fd= pipe_comandes[0];

	timev.tv_sec = 0;
	timev.tv_usec = 10;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	a = select(fd +1, &readfds, NULL, NULL, &timev);
	if( a < 0 )
	{
        print_with_time("ERROR => Select que llegeix del procés pare les comandes.\n");
		exit(-1);
	} else if(FD_ISSET(fd, &readfds)){
		read(pipe_comandes[0], comanda, sizeof(comanda));
		if(strcmp(comanda, "quit\n") == 0)
		{
            print_if_debug(debug, "S'ha executat la comanda 'quit'.");
			print_if_debug(debug, "Finalitzat procés per gestionar alives.");
			return 1;
		} else if(strcmp(comanda, "send-conf\n") == 0)
        {
            print_if_debug(debug, "S'executarà comanda 'send-conf'.");
            return 2;
        } else if(strcmp(comanda, "get-conf\n") == 0)
        {
            print_if_debug(debug, "S'executarà comanda 'get-conf'.");
            return 3;
        }
	}
	return 0;
}

/*
 * Funció: es_servidor_correcte
 * -----------------
 * Mira si el servidor és el mateix que al que rebem al REGISTER_ACK
 */
int es_servidor_correcte(struct paquet_udp paquet_recv, struct info_serv info_s){
	return !strcmp(paquet_recv.equip, info_s.nom) && !strcmp(paquet_recv.mac, info_s.mac) && !strcmp(paquet_recv.random_number, info_s.aleatori);
}

/*
 * Funció: control_stop
 * -----------------
 * Mira si s'ha superat el màxim de ALIVE_ACK no rebuts.
 */
int control_stop(int count_no_alive_ack)
{
	if(U <= count_no_alive_ack)
	{
		print_with_time("ERROR => No s'han rebut %i ALIVE_ACK", U, count_no_alive_ack);
		return 1;
	}
	return 0;
}

/*
 * Funció: escriure_paquet
 * -----------------
 * Escriu el paquet amb la pdu udp segons el tipus, el client i l'aleatori. 
 */
struct paquet_udp escriure_paquet(int type ,struct client c, char * random)
{
	struct paquet_udp p;
	memset(&p,0, sizeof(p));
	p.type= (unsigned char) type;
	strcpy(p.equip, c.equip);
	strcpy(p.mac, c.mac);
	strcpy(p.random_number, random);
	strcpy(p.dades, "\0");
	return p;
}