#include <stdio.h>

#include "client_funcions.h"

/*	
 * Funció: actualitzar_hora
 * -----------------
 * Guarda el temps actual en H:M:S a la String hora.
 */
void actualitzar_hora(struct t *temps)
{
	temps->t = time(NULL);
	temps->tm=localtime(&temps->t);
	strftime(temps->hora, 15, "%H:%M:%S", temps->tm);
}

void print_with_time(const char *fmt, ...){
	struct t temps;
	va_list args;

	va_start(args, fmt);
	actualitzar_hora(&temps);
	printf("%s: ", temps.hora);
	print_string(fmt, args);
}

void print_string(const char *fmt, va_list args)
{
	char *string;
	int i;
	while( *fmt != '\0')
	{
		if(*fmt == '%' && *(fmt+1) == 's')
		{
			++fmt;
			string = va_arg(args,char*);
			printf("%s", string);
		} else if(*fmt == '%' && *(fmt+1)== 'i')
		{
			++fmt;
			i = va_arg(args,int);
			printf("%i", i);
		} else {
			printf("%c", *fmt);
		}
		++fmt;
	}
	printf("\n");
}

/*
 * Funció: error_arxiu_configuracio
 * -----------------
 * Imprimeix per pantalla que no s'ha pogut obrir l'arxiu de configuració i finalitza el programa.
 */
void error_arxiu_configuracio(char arxiu[])
{
	print_with_time("ERROR =>  No es pot obrir l'arxiu de configuració: %s\n", arxiu);
	exit(-1);
}

/*
 * Funció: print_if_debug
 * -----------------
 * En cas que estigui en mode debug, printeja el text.
 */
void print_if_debug(int debug, const char *fmt, ...)
{
	va_list	args;
	struct t temps;

	if(debug == 1)
	{
	    actualitzar_hora(&temps);
	    printf("%s: DEBUG => ", temps.hora);
	    va_start(args,fmt);
		print_string(fmt, args);
	}
}