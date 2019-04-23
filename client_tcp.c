#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include "client_funcions.h"

#define W 4

void send_conf_command(int debug, struct server s, struct info_serv inf_s, struct client c, char aleatori[7], char boot_file[])
{
    int pid;
    struct server s_tcp;

    pid = fork();
    if(pid == 0)
    {
        print_if_debug(debug, "Fill per la connexió TCP creat!");
        strcpy(s_tcp.server, s.server);
        s_tcp.server_port = inf_s.port_tcp;
        send_conf(debug, s_tcp, c, aleatori, boot_file, inf_s);
        exit(1);
    }
}

void send_conf(int debug, struct server s, struct client c, char aleatori[7], char boot_file[], struct info_serv inf_s)
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
    comunicacio_enviar_fitxer(debug, fd, addr_serv, paquet, file, c, inf_s);
    fclose(file);
    print_if_debug(debug, "Es tanca fitxer %s", boot_file);
}

void comunicacio_enviar_fitxer(int debug, int fd, struct sockaddr_in addr_serv, struct paquet_tcp paquet, FILE * file, struct client c, struct info_serv inf_s)
{
    int a;
    struct timeval timev;
    fd_set rfds;

    timev.tv_sec = W;
    timev.tv_usec = 0;

    a = connect(fd, (struct sockaddr *) &addr_serv, sizeof(addr_serv));
    if (a < 0)
    {
        print_with_time("ERROR => No s'ha pogut connectar a l'adreça del servidor.");
    }
    write(fd, &paquet, sizeof(paquet));
    print_with_time("MSG.  => Sol·licitud d'enviament d'arxiu de configuració al servidor");
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
            if(dades_pdu_tcp_correctes(paquet, inf_s)){
                enviar_arxiu_configuracio(debug, fd, paquet, file, c); 
            } else {
                print_if_debug(debug, "S'ha rebut ACK incorrecte");
            }
        } else {
            print_if_debug(debug, "No s'ha rebut ACK. Es tanca comunicació.");
        }
    } else {
        print_if_debug(debug, "No s'ha rebut cap ACK");
    }
    close(fd);
}

int dades_pdu_tcp_correctes(struct paquet_tcp paquet, struct info_serv inf_s)
{
    if(strcmp(paquet.equip, inf_s.nom) == 0 && strcmp(paquet.mac, inf_s.mac) == 0 && strcmp(paquet.random_number, inf_s.aleatori) == 0)
    {
        return 1;
    }
    return 0;
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

void get_conf_command(int debug, struct server s, struct info_serv inf_s, struct client c, char aleatori[7], char boot_file[])
{    
    int pid;
    struct server s_tcp;

    pid = fork();
    if(pid == 0)
    {
        print_if_debug(debug, "Fill per la connexió TCP creat!");
        strcpy(s_tcp.server, s.server);
        s_tcp.server_port = inf_s.port_tcp;
        get_conf(debug, s_tcp, c, aleatori, boot_file, inf_s);
        exit(1);
    }
}

void get_conf(int debug, struct server s, struct client c, char aleatori[7], char boot_file[], struct info_serv inf_s)
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

    file = fopen(boot_file, "w+");

    memset(&paquet,0, sizeof(paquet));
    paquet.type= (unsigned char) 0x30;
    strcpy(paquet.equip, c.equip);
    strcpy(paquet.mac, c.mac);
    strcpy(paquet.random_number, aleatori);
    strcpy(paquet.dades, "\0");


    fseek(file, 0L, SEEK_END);
    print_if_debug(debug, "Obert fitxer %s de %i B.", boot_file, ftell(file));
    sprintf(paquet.dades, "%s,%ld", boot_file, ftell(file));
    comunicacio_rebre_fitxer(debug, fd, addr_serv, paquet, file, c, inf_s);
    fclose(file);
    print_if_debug(debug, "Es tanca fitxer %s", boot_file);
}

void comunicacio_rebre_fitxer(int debug, int fd, struct sockaddr_in addr_serv, struct paquet_tcp paquet, FILE * file, struct client c, struct info_serv inf_s)
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
    print_with_time("MSG.  => Sol·licitud de recepció d'arxiu de configuració al servidor");
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
            if(dades_pdu_tcp_correctes(paquet, inf_s)){
                rebre_arxiu_configuracio(debug, fd, paquet, file, c); 
            } else {
                print_if_debug(debug, "S'ha rebut ACK incorrecte");
            }
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
    int end = 0;
    fseek(file, 0, SEEK_SET);
    while(!end)
    {
        if(read(fd, &paquet, sizeof(paquet)) > 0)
        {
    	    print_if_debug(debug, "Rebut: bytes=%i, comanda=%s, nom=%s, mac=%s, alea=%s, dades=%s", sizeof(paquet),tipus_pdu(paquet.type), paquet.equip, paquet.mac, paquet.random_number, paquet.dades);
            if(paquet.type == 0x34)
            {
                fprintf(file, "%s", paquet.dades);
            } else if(paquet.type == 0x35)
            {
                print_if_debug(debug, "Fi de get_conf");
                end = 1;
            } else
            {
                print_if_debug(debug, "No s'ha rebut GET_DATA");
            }
        } else
        {
            fprintf(file, "\n");
        }
    }
    while(fgets(linea, 150, file))
    {
        fprintf(file, "\n");
    }
    if(!end)
    {
        print_with_time("MSG.  => No s'ha rebut GET_END.");
    }

    paquet.type = (unsigned char) 0x25;
    strcpy(paquet.dades, "");
    write(fd, &paquet, sizeof(paquet));
    print_with_time("MSG.  => Finalitzat recepció d'arxiu de configuració al servidor");

}
