#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct args
{
    FILE *fitxer_soft;
    char fitxer_equip[10];
    int debug;
};

struct t
{
    time_t t;
	struct tm *tm;
	char hora[15];
};

struct server
{
    char server[20];
    int server_port;
};
struct client
{
    char equip[7];
    char mac[13];
};
struct paquet_udp
{
    unsigned char type;
    char equip[7];
    char mac[13];
    char random_number[7];
    char dades[50];
};

struct paquet_tcp
{
    unsigned char type;
    char equip[7];
    char mac[13];
    char random_number[7];
    char dades[150];
};

struct temporitzadors
{
    int n;
    int t;
    int p;
    int q;
    int numIntents;
};
struct info_serv
{
    char nom[7];
    char ip[10];
    char mac[13];
    char aleatori[7];
    int port_tcp;
};