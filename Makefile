all: server client
server: temp/servidor
client: temp/cliente

temp/servidor: obj/servidor.o
	gcc -g obj/servidor.o -o temp/servidor
obj/servidor.o: src/servidor.c
	gcc -Wall -g -c src/servidor.c -o obj/servidor.o
temp/cliente: obj/cliente.o
	gcc -g obj/cliente.o -o temp/cliente
obj/cliente.o: src/cliente.c
	gcc -Wall -g -c src/cliente.c -o obj/cliente.o
clean:
	@ rm obj/* 
	@ rm temp/*