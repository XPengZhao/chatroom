#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define BACKLOG 100
#define MAXDATASIZE  2048

/*聊天室成员信息*/
typedef struct Member{
    char name[100];
    int sockfd;
    struct Member *next;
} Member;

/*聊天室成员链表*/
typedef struct Room{
    Member *head;
    int n;
}Room;

typedef struct File_info{
    int filesize;
    char filename[100];
}File_info;

int StartServer(void);
void *pthread_func(void *fd);
void broadcastmsg(int fd, char recv_buf[]);
void recv_file(int fd);


