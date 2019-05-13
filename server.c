#include "server.h"

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

    //serveraddr用来记录给定的IP和port信息
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
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
        if(*clientfd!=-1){
            printf("\n=====================客户端链接成功=====================\n");
            printf("IP = %s:PORT = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
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
    char buf[MAXDATASIZE];
    client_sockfd=*(int *)fd;

    while(1){
        memset(buf, '\0', MAXDATASIZE/sizeof (char));
        int recv_length = recv(client_sockfd, buf, MAXDATASIZE/sizeof (char), 0);
        if(recv_length == 0)
        {
            printf("client has closed!\n");
            break;
        }            
        else if(recv_length == -1){
             perror("recv");  
             exit(EXIT_FAILURE); 
        }

        if(strcmp(buf,"exit") == 0){
            break;
        }
 
        printf("client say[%d]: ", recv_length);
        fputs(buf, stdout);
        //memset(buf, '\0', MAXDATASIZE/sizeof (char));
        //printf("input: ");
        //fgets(buf, sizeof(buf), stdin);
        //send(client_sockfd, buf, recv_length, 0);
    }
    close(client_sockfd);
    free(fd);
    pthread_exit(NULL);

 

}
int main(void){

    StartServer();
    return 0;
}
