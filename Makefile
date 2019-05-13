all:Server Client 
Server:server.c server.h
	gcc -o server server.c -lpthread
Client:client.c
	gcc -o client client.c
clean:
	rm server client
