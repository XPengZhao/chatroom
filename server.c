#include "server.h"

/*聊天室成员信息*/
typedef struct Member{
    char name[100];
    char password[100];
    int sockfd;
    struct Member *next;
} Member;

/*聊天室成员链表*/
typedef struct Room{
    Member *head;
    int n;
}Room;

Room room1={ NULL,0 };

/**
  * @brief 创建结点
  * @param name-->姓名，pwd-->密码，sockfd-->客户端Socket描述符
  * @retval 该客户的成员信息
  * @details None
  */
Member *CreateNode(char name[], char pwd[], int sockfd)
{
    Member *p = (Member *)malloc(sizeof(Member));
    strcpy(p->name,name);
    strcpy(p->password,pwd);
    p->sockfd = sockfd;
    return p;
}

/**
  * @brief 添加结点
  * @param room-->聊天室链表;  usr-->成员信息结点
  */
void AddOnlineUsr(Room *room, Member *usr)
{
    Member *p = NULL;
    if(room->n == 0){
        room->head=usr;
        room->n++;
    }
    else{
        for(p = room->head; p->next != NULL; p=p->next);
        p->next = usr;
    }
}

/**
  * @brief 删除结点
  * @param room-->聊天室链表;  usr-->成员信息结点
  */
void DeleteOnlineUsr(Room *room, Member *usr){
    Member *p1 = NULL, *p2 = NULL;
    for(p1 = room->head; p1->next != NULL; p1=p1->next){
        if(p1->sockfd == usr->sockfd)
            break;
        p2 = p1;
    }
    if( p1 == room->head){
        room->head = p1->next;
        free(p1);
    }
    else{
        p2->next = p1->next;
        free(p1);
    }
}

/**
  * @brief 由姓名查找聊天室成员
  * @param name-->成员姓名指针
  * @
  */
Member *searchbyname(char *name)
{

}


void GetUserInfo(char *name, char *pwd, int client_sockfd)
{
    char send_buf[MAXDATASIZE]={'\0'};
    strcpy(send_buf, "name?");
    send(client_sockfd, send_buf, sizeof(send_buf), 0);
    recv(client_sockfd, name, MAXDATASIZE, 0);
    strcpy(send_buf, "password?");
    send(client_sockfd, send_buf, sizeof(send_buf), 0);
    recv(client_sockfd, pwd, MAXDATASIZE, 0);
}

/**
  * @brief 启动服务器端服务，等待客户端连接
  * @param None
  * @retval successful-->Socket文件描述符;    failed-->-1
  * @details 1. 通配地址 INADDR_ANY 表示IP地址为 0.0.0.0 
  *             内核在套接字被连接后选择一个本地地址。
  *          2. 指派为通配端口 0，
  *             调用 bind 函数后内核将任意选择一个临时端口
  */ 
int StartServer(void)
{

    int serverfd;
    int * clientfd;
    struct sockaddr_in serveraddr,clientaddr;

    serverfd = socket(AF_INET, SOCK_STREAM, 0);  //创建一个socket描述符
    printf("serverfd=%d\n", serverfd);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT); 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(serverfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(serverfd, BACKLOG);                   //创建一个监听队列，保存用户的请求连接信息（ip、port、protocol)

    printf("======bind success,waiting for client's request======\n");
    socklen_t client_len = sizeof(clientaddr);
    while(1)
    {
        pthread_t id;
        clientfd = (int *)malloc(sizeof(int));
        //accept函数从listen函数维护的监听队列里取一个客户连接请求处理
        *clientfd = accept(serverfd, (struct sockaddr*)&clientaddr, &client_len);
        
        if(*clientfd!=-1){
            printf("\n=====================客户端链接成功=====================\n");
            printf("IP = %s:PORT = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        }
        else{
            printf("\n=====================客户端连接失败=====================\n");
            continue;
        }
        if(pthread_create(&id, NULL, thread_fun, clientfd)!=0){         //创建子线程 
            perror("pthread_create");
            break;
        }
    }
    shutdown(*clientfd,2);
    shutdown(serverfd,2);
    return 0;
}

/**
  * @brief 客户端握手成功后的线程处理函数
  * @param 客户端socket文件描述符
  * @retval None
  * @detail None
  */
void *thread_fun(void *fd){
    int client_sockfd;
    char recv_buf[MAXDATASIZE] = {'\0'}, send_buf[MAXDATASIZE] = {'\0'};
    client_sockfd=*(int *)fd;
    char name[MAXDATASIZE] = {'\0'}, pwd[MAXDATASIZE] = {'\0'};
    Member *usr = NULL;
    strcpy(send_buf,"welcome to chat room\n");    
    send(client_sockfd, send_buf, sizeof(send_buf), 0);

    GetUserInfo(name, pwd, client_sockfd);
    usr = CreateNode(name, pwd, client_sockfd);
    AddOnlineUsr(&room1, usr);
    while(1){
        memset(recv_buf, '\0', MAXDATASIZE/sizeof (char));
        //接收缓冲区中没有数据或者协议正在接收数据，那么recv就一直等待，直到协议把数据接收完毕
        int recv_length = recv(client_sockfd, recv_buf, sizeof (recv_buf), 0);
        if(recv_length == 0)
        {
            printf("client %d has closed!\n",client_sockfd);
            break;
        }            
        else if(recv_length == -1){
             perror("recv");  
             exit(EXIT_FAILURE); 
        }

        if(strcmp(recv_buf,"exit\n") == 0){
            printf("client %d has closed!\n",client_sockfd);
            break;
        }
 
        printf("client[%d] say: ", client_sockfd);
        broadcastmsg(client_sockfd,recv_buf);
        fputs(recv_buf, stdout);
        fflush(stdout);
        //unix上标准输入输出都是带有缓存的,当遇到行刷新标志或者该缓存已满的情况下，才会把缓存的数据显示到终端设备上。
        //ANSI C中定义换行符'\n'可以认为是行刷新标志。所以，printf函数没有带'\n'是不会自动刷新输出流，直至缓存被填满。
        //操作系统为减少 IO操作 所以设置了缓冲区.  等缓冲区满了再去操作IO. 这样是为了提高效率。
    }
    close(client_sockfd);
    free(fd);
    pthread_exit(NULL);
}

/**
  * @brief 将客户端发送的消息广播到全聊天室
  * @param fd-->socket描述符
  * @retval
  * @details
  */
void broadcastmsg(int fd, char recv_buf[])
{
    Member *p = NULL;
    for(p=room1.head; p != NULL; p=p->next){
        if(p->sockfd != fd){
            send(p->sockfd, recv_buf, MAXDATASIZE, 0);
        }
    }
}


int main(void){

    StartServer();
    return 0;
}
