CFLAGS = -ansi -pedantic -Wall

# Regles
programa: client_main.o client_prints.o
	gcc -o client client_main.o client_prints.o
	@rm -f client_main.o
	@rm -f client_prints.o

client_main.o: client_main.c
	gcc -c client_main.c -ansi -pedantic -Wall

client_prints.o: client_prints.c
	gcc -c client_prints.c -ansi -pedantic -Wall
	
clean: client
	@rm -f client
	@echo "S'ha esborrat l'executable \"client\""