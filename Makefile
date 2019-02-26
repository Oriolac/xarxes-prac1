CFLAGS = -ansi -pedantic -Wall

# Regles
programa: client_main.o 
	gcc -o client client_main.o 
	@rm -f client_main.o

client_main.o: client_main.c
	gcc -c client_main.c -ansi -pedantic -Wall
	
clean: client
	@rm -f client
	@echo "S'ha esborrat l'executable \"client\""