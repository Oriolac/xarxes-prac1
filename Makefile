CFLAGS = -ansi -pedantic -Wall

# Regles
programa: client_main.o client_prints.o client_parametres.o client_udp.o
	gcc -o client client_main.o client_prints.o client_parametres.o client_udp.o
	@rm -f client_parametres.o
	@rm -f client_main.o
	@rm -f client_prints.o
	@rm -f client_udp.o

client_main.o: client_main.c
	gcc -c client_main.c -ansi -pedantic -Wall

client_prints.o: client_prints.c
	gcc -c client_prints.c -ansi -pedantic -Wall

client_parametres.o: client_parametres.c
	gcc -c client_parametres.c -ansi -pedantic -Wall

client_udp.o: client_udp.c
	gcc -c client_udp.c -ansi -pedantic -Wall
	
clean: client
	@rm -f client
	@echo "S'ha esborrat l'executable \"client\""