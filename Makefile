
#Regles
all: programa

programa: client_main.c client_prints.c client_parametres.c client_udp.c
	gcc -o client client_main.c client_prints.c client_parametres.c client_udp.c -ansi -pedantic -Wall
clean: client
	@rm -f client
	@echo "S'ha esborrat l'executable \"client\""