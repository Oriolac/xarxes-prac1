#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct args
{
    FILE *fitxerSoft;
    FILE *fitxerEquip;
    int debug;
};

struct t
{
    time_t t;
	struct tm *tm;
	char hora[15];
};

struct udpSock
{
    int fd;
};

struct client
{
    char nom[15];
    char mac[13];
    char server[15];
    int serverPort;
};