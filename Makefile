all:Server Client 
Server:server.c server.h
	gcc -g -o server server.c -lpthread
Client:client.c
	gcc -g -o client client.c -lpthread
clean:
	rm server client
