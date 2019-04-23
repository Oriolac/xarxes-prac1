#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include "client_funcions.h"

#define W 4

void send_conf_command(int debug, char host[20], int port, struct client c, char aleatori[7], char boot_file[])
{
    int pid;
    struct server s;

    pid = fork();
    if(pid == 0)
    {
        print_if_debug(debug, "Fill per la connexió TCP creat!");
        strcpy(s.server, host);
        s.server_port = port;
        send_conf(debug, s, c, aleatori, boot_file);
        exit(1);
    }
}

void send_conf(int debug, struct server s, struct client c, char aleatori[7], char boot_file[])
{
    struct sockaddr_in addr_serv;
    struct paquet_tcp paquet;
    int fd;
    FILE * file;

    fd = socket(AF_INET,SOCK_STREAM, 0);
    if(fd < 0)
    {
        print_with_time("ERROR =>  No s'ha pogut crear socket.");
        exit(-1);
    }
    print_if_debug(debug, "Realitzat el socket TCP.");

    addr_serv = addr_servidor(s);
    print_if_debug(debug, "Emplenada l'adreça del servidor i el seu port.");

    file = fopen(boot_file, "r");

    memset(&paquet,0, sizeof(paquet));
    paquet.type= (unsigned char) 0x20;
    strcpy(paquet.equip, c.equip);
    strcpy(paquet.mac, c.mac);
    strcpy(paquet.random_number, aleatori);
    strcpy(paquet.dades, "\0");


    fseek(file, 0L, SEEK_END);
    print_if_debug(debug, "Obert fitxer %s de %i B.", boot_file, ftell(file));
    sprintf(paquet.dades, "%s,%ld", boot_file, ftell(file));
    comunicacio_enviar_fitxer(debug, fd, addr_serv, paquet, file, c);
    fclose(file);
    print_if_debug(debug, "Es tanca fitxer %s", boot_file);
}

void comunicacio_enviar_fitxer(int debug, int fd, struct sockaddr_in addr_serv, struct paquet_tcp paquet, FILE * file, struct client c)
{
    int a;
    struct timeval timev;
    fd_set rfds;

    timev.tv_sec = W;
    timev.tv_usec = 0;

    a = connect(fd, (struct sockaddr *) &addr_serv, sizeof(addr_serv));
    if (a < 0)
    {
        print_with_time("ERROR => No s'ha pogut connectar a l'adreça del servidor.\n");
    }
    write(fd, &paquet, sizeof(paquet));
	print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    a = select(fd + 1, &rfds, NULL, NULL, &timev);
    if (a < 0)
    {
        print_with_time("ERROR => Select que espera un SEND_ACK.\n");
        close(fd);
        exit(-1);
    } else if(FD_ISSET(fd, &rfds))
    {
        read(fd, &paquet, sizeof(paquet));
		print_if_debug(debug,"S'ha rebut el paquet: tipus=%s, nom=%s, mac=%s, alea=%s, dades=%s", tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
        if(paquet.type == 0x21)
        {
            enviar_arxiu_configuracio(debug, fd, paquet, file, c); 
        } else {
            print_if_debug(debug, "No s'ha rebut ACK. Es tanca comunicació.");
        }
    } else {
        print_if_debug(debug, "No s'ha rebut cap ACK");
    }
    close(fd);
}

void enviar_arxiu_configuracio(int debug, int fd, struct paquet_tcp paquet, FILE * file, struct client c)
{
    char linea[150];
    paquet.type= (unsigned char) 0x24;
    strcpy(paquet.equip, c.equip);
    strcpy(paquet.mac, c.mac);
    fseek(file, 0, SEEK_SET);
    while(fgets(linea, 150, file))
    {
        strcpy(paquet.dades, linea);
        write(fd, &paquet, sizeof(paquet));
    	print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
        memset(linea,0, sizeof(linea));
    }
    paquet.type = (unsigned char) 0x25;
    strcpy(paquet.dades, "");
    write(fd, &paquet, sizeof(paquet));
    print_with_time("MSG.  => Finalitzat enviament d'arxiu de configuració al servidor");
}

void get_conf_command(int debug, char host[20], int port, struct client c, char aleatori[7], char boot_file[])
{    
    int pid;
    struct server s;

    pid = fork();
    if(pid == 0)
    {
        print_if_debug(debug, "Fill per la connexió TCP creat!");
        strcpy(s.server, host);
        s.server_port = port;
        get_conf(debug, s, c, aleatori, boot_file);
        exit(1);
    }
}

void get_conf(int debug, struct server s, struct client c, char aleatori[7], char boot_file[])
{

    struct sockaddr_in addr_serv;
    struct paquet_tcp paquet;
    int fd;
    FILE * file;

    fd = socket(AF_INET,SOCK_STREAM, 0);
    if(fd < 0)
    {
        print_with_time("ERROR =>  No s'ha pogut crear socket.");
        exit(-1);
    }
    print_if_debug(debug, "Realitzat el socket TCP.");

    addr_serv = addr_servidor(s);
    print_if_debug(debug, "Emplenada l'adreça del servidor i el seu port.");

    file = fopen(boot_file, "r+");

    memset(&paquet,0, sizeof(paquet));
    paquet.type= (unsigned char) 0x30;
    strcpy(paquet.equip, c.equip);
    strcpy(paquet.mac, c.mac);
    strcpy(paquet.random_number, aleatori);
    strcpy(paquet.dades, "\0");


    fseek(file, 0L, SEEK_END);
    print_if_debug(debug, "Obert fitxer %s de %i B.", boot_file, ftell(file));
    sprintf(paquet.dades, "%s,%ld", boot_file, ftell(file));
    comunicacio_rebre_fitxer(debug, fd, addr_serv, paquet, file, c);
    fclose(file);
    print_if_debug(debug, "Es tanca fitxer %s", boot_file);
}

void comunicacio_rebre_fitxer(int debug, int fd, struct sockaddr_in addr_serv, struct paquet_tcp paquet, FILE * file, struct client c)
{
    int a;
    struct timeval timev;
    fd_set rfds;

    timev.tv_sec = W;
    timev.tv_usec = 0;

    a = connect(fd, (struct sockaddr *) &addr_serv, sizeof(addr_serv));
    if (a < 0)
    {
        print_with_time("ERROR => No s'ha pogut connectar a l'adreça del servidor.\n");
    }
    write(fd, &paquet, sizeof(paquet));
	print_if_debug(debug, "Enviat: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    a = select(fd + 1, &rfds, NULL, NULL, &timev);
    if (a < 0)
    {
        print_with_time("ERROR => Select que espera un SEND_ACK.\n");
        close(fd);
        exit(-1);
    } else if(FD_ISSET(fd, &rfds))
    {
        read(fd, &paquet, sizeof(paquet));
		print_if_debug(debug,"S'ha rebut el paquet: tipus=%s, nom=%s, mac=%s, alea=%s, dades=%s", tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
        if(paquet.type == 0x31)
        {
            rebre_arxiu_configuracio(debug, fd, paquet, file, c); 
        } else {
            print_if_debug(debug, "No s'ha rebut ACK. Es tanca comunicació.");
        }
    } else {
        print_if_debug(debug, "No s'ha rebut cap ACK");
    }
    close(fd);
}

void rebre_arxiu_configuracio(int debug, int fd, struct paquet_tcp paquet, FILE * file, struct client c)
{
    char linea[150];
    fseek(file, 0, SEEK_SET);
    while(fgets(linea, 150, file))
    {
        if(read(fd, &paquet, sizeof(paquet)) > 0)
        {
    	    print_if_debug(debug, "Rebut: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
            if(paquet.type == 0x34)
            {
                fprintf(file, "%s", paquet.dades);
            } else
            {
                print_if_debug(debug, "No s'ha rebut GET_DATA");
            }
        } else
        {
            fprintf(file, "\n");
        }
    }

    paquet.type = (unsigned char) 0x25;
    strcpy(paquet.dades, "");
    write(fd, &paquet, sizeof(paquet));
    print_with_time("MSG.  => Finalitzat repeció d'arxiu de configuració al servidor");

}
