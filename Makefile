all: server client
server: bin/servidor
client: bin/cliente

bin/servidor: obj/servidor.o
	gcc -g obj/servidor.o -o bin/servidor
obj/servidor.o: src/servidor.c
	gcc -Wall -g -c src/servidor.c -o obj/servidor.o
bin/cliente: obj/cliente.o
	gcc -g obj/cliente.o -o bin/cliente
obj/cliente.o: src/cliente.c
	gcc -Wall -g -c src/cliente.c -o obj/cliente.o
clean:
	rm obj/* tmp/* bin/{cliente,servidor}