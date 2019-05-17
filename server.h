#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

int StartServer(void);
void *thread_fun(void *fd);
void broadcastmsg(int fd, char recv_buf[]);

#define PORT 8888
#define BACKLOG 10
#define MAXDATASIZE  2048

