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

struct server
{
    char server[20];
    int serverPort;
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

struct temporitzadors
{
    int n;
    int t;
    int m;
    int p;
    int s;
    int q;
};