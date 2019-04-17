#include <sys/types.h>
#include <unistd.h>
#include <string.h>
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

    paquet = escriure_paquet(0x20, c,aleatori);
    fseek(file, 0L, SEEK_END);
    print_if_debug(debug, "Obert fitxer %s de %i B.", boot_file, ftell(file));
    sprintf(paquet.dades, "%s,%ld", boot_file, ftell(file));
    comunicacio_enviar_fitxer(debug, fd, addr_serv, paquet, file);
    fclose(file);
}

void comunicacio_enviar_fitxer(int debug, int fd, struct sockaddr_in addr_serv, struct paquet_tcp paquet, FILE * file)
{
    int a;
    struct timeval temps;
    fd_set rfds;

    timev.tv_sec = W;
    timev.tv_usec = 0;

    a = connect(fd, addr_serv, sizeof(addr_serv));
    if (a < 0)
    {
        print_with_time("ERROR => No s'ha pogut connectar a l'adreça del servidor.\n");
    }
    write(fd, paquet, sizeof(paquet));

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    a = select(fd + 1, &rfds, NULL, NULL, NULL);
    if (a < 0)
    {
        print_with_time("ERROR => Select que espera un SEND_ACK.\n");
        close(fd);
        exit(-1);
    } else if(FD_ISSET(fd, &rfds))
    {
        read(fd, paquet, sizeof(paquet));
    } else {
        print_if_debug(debug, "No s'ha rebut cap ACK");
    }
    close(fd);

}