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
Member *searchbyname(Room *room, char *name)
{
    Member *p = room->head;
    for (;p != NULL; p = p->next)
        if (strcmp(name, p->name) == 0)
            return p;
    return NULL;
}

/**
  * @brief 由sockfd查找聊天室成员
  * @param sorkfd-->成员姓名指针
  * @
  */
 Member *searchbysockfd(Room *room, int sockfd)
{
    Member *p = room->head;
    for (;p != NULL; p = p->next)
        if (p->sockfd == sockfd)
            return p;
    return NULL;
}


int GetUserInfo(char *name, char *pwd, int client_sockfd)
{
    char send_buf[MAXDATASIZE]={'\0'};
    strcpy(send_buf, "name?");
    send(client_sockfd, send_buf, sizeof(send_buf), 0);
    recv(client_sockfd, name, MAXDATASIZE, 0);
    if(name[strlen(name)-1] == '\n')
        name[strlen(name)-1] = '\0';
    strcpy(send_buf, "password?");
    send(client_sockfd, send_buf, sizeof(send_buf), 0);
    recv(client_sockfd, pwd, MAXDATASIZE, 0);
    if (searchbyname(&room1, name))
    {
        strcpy(send_buf, "used");
        send(client_sockfd, send_buf, sizeof(send_buf), 0);
        return 1;
    }
    else
    {
        strcpy(send_buf, "ok");
        send(client_sockfd, send_buf, sizeof(send_buf), 0);
        return 0;
    }
    
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

    serveraddr.sin_port = htons(PORT); 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(serverfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    //创建一个监听队列，保存用户的请求连接信息（ip、port、protocol)
    listen(serverfd, BACKLOG);

    printf("======bind success,waiting for client's request======\n");
    //让操作系统回填client的连接信息（ip、port、protocol）
    socklen_t client_len = sizeof(clientaddr);
    while(1)
    {
        pthread_t id;
        clientfd = (int *)malloc(sizeof(int));
        //accept函数从listen函数维护的监听队列里取一个客户连接请求处理
        *clientfd = accept(serverfd, (struct sockaddr*)&clientaddr, &client_len);
        
        /*
         * 此处进行用户身份验证
         * */

        if(*clientfd!=-1){
            printf("\n=====================客户端链接成功=====================\n");
            printf("IP = %s:PORT = %d, clientfd = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), *clientfd);
        }
        else{
            printf("\n=====================客户端连接失败=====================\n");
            continue;
        }
        if(pthread_create(&id, NULL, recv_data, clientfd)!=0){         //创建子线程 
            perror("pthread_create");
            break;
        }
    }
    shutdown(*clientfd,2);
    shutdown(serverfd,2);
    return 0;
}

/**
  * @brief 从客户端接受数据
  * @param 客户端socket文件描述符
  * @retval None
  * @detail None
  */
void *recv_data(void *fd){
    int client_sockfd;
    char recv_buf[MAXDATASIZE] = {'\0'}, send_buf[MAXDATASIZE] = {'\0'};
    client_sockfd=*(int *)fd;
    char name[MAXDATASIZE] = {'\0'}, pwd[MAXDATASIZE] = {'\0'}, temp[MAXDATASIZE] = {'\0'};
    Member *usr = NULL;
    if (room1.n < 100)
        strcpy(send_buf,"WELCOME!\n");
    else
        strcpy(send_buf, "FULL!\n");
    send(client_sockfd, send_buf, sizeof(send_buf), 0);

    while(GetUserInfo(name, pwd, client_sockfd));
    usr = CreateNode(name, pwd, client_sockfd);
    AddOnlineUsr(&room1, usr);
    snprintf(temp, MAXDATASIZE, "%s has join the chatroom\n", searchbysockfd(&room1, client_sockfd)->name);
    broadcastmsg(client_sockfd, temp);

    while(1){
        memset(recv_buf, '\0', MAXDATASIZE/sizeof (char));
        //接收缓冲区中没有数据或者协议正在接收数据，那么recv就一直等待，直到协议把数据接收完毕
        int recv_length = recv(client_sockfd, recv_buf, sizeof (recv_buf), 0);
        if(strncmp(recv_buf, "/exit", strlen("/exit")) == 0 || recv_length == 0)
        {
            printf("client %s has closed!\n", searchbysockfd(&room1, client_sockfd)->name);
            snprintf(temp, MAXDATASIZE, "%s has quited the chatroom\n", searchbysockfd(&room1, client_sockfd)->name);
            broadcastmsg(client_sockfd, temp);
            break;
        }            
        else if(recv_length == -1){
             perror("recv");  
             exit(EXIT_FAILURE); 
        }
        else if(strncmp(recv_buf, "/file", strlen("/file")) == 0)
        {
        //开始文件的读写操作
            char buf[MAXDATASIZE];
            memset(buf,0x00,sizeof(buf));
            int filefd = open("copy.txt",O_WRONLY |O_CREAT |O_TRUNC,0777);
            while(1)
            {
                int leng = recv(client_sockfd,buf,sizeof(buf),0);
                if(leng == 0)
                {
                    printf("Opposite have close the socket.\n"); 
                    break; //表示文件已经读到了结尾,也意味着客户端关闭了socket
                }
                if(leng == -1 && errno == EINTR)
                    continue;
                if(leng == -1 )
                    break; //表示出现了严重的错误
                write(filefd,buf,leng);                                                                                                                                                                                                                        
            }  
            //若文件的读写已经结束,则关闭文件描述符
            close(filefd);
            break;
        }

        //if(strcmp(recv_buf,"exit") == 0){
        //     break;
        //}
 
    	printf("%s say: ", searchbysockfd(&room1, client_sockfd)->name);
        broadcastmsg(client_sockfd,recv_buf);
        fputs(recv_buf, stdout);
        fputs("\n", stdout);
        fflush(stdout);
        //unix上标准输入输出都是带有缓存的,当遇到行刷新标志或者该缓存已满的情况下，才会把缓存的数据显示到终端设备上。
        //ANSI C中定义换行符'\n'可以认为是行刷新标志。所以，printf函数没有带'\n'是不会自动刷新输出流，直至缓存被填满。
        //ANSI C中定义换行符'\n'可以认为是行刷新标志。所以，printf函数没有带'\n'是不会自动刷新输出流，直至缓存被填满。
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
    char temp1[MAXDATASIZE / 2], temp2[MAXDATASIZE];
    strcpy(temp1, recv_buf);
    Member *p = NULL, *q = NULL;
    for (q = room1.head; q; q = q->next)
        if (q->sockfd == fd)
            break;
    snprintf(temp2, MAXDATASIZE, "client %s: %s", q->name, temp1);
    for(p=room1.head; p; p=p->next){
        if(p->sockfd != fd)
        {
            send(p->sockfd, temp2, MAXDATASIZE, 0);
        }
    }
}


int main(void){

    StartServer();
    return 0;
}
