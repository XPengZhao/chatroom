#include "client.h"
#include <sys/stat.h>


typedef struct File_info{
    int filesize;
    char filename[100];
}File_info;

int get_filesize(char *filename)
{
    struct stat statbuf;
    stat(filename, &statbuf);
    int size = statbuf.st_size;
    
    return size;
}

void send_file(int fd){
    char path[MAXDATASIZE], buf[MAXDATASIZE];
    File_info file_info;
    int server_sockfd = fd;
    memset(buf, 0, sizeof(buf));
    memset(path, 0, sizeof(path));
    printf("path: ");
    fgets(path, MAXDATASIZE, stdin);
    if(path[strlen(path)-1] == '\n')
        path[strlen(path)-1] = '\0';
    fflush(stdin);                     //清除输入缓存 

    file_info.filesize = get_filesize(path);
    strcpy(file_info.filename,path);
    memcpy(buf, &file_info, sizeof(file_info));
    send(server_sockfd, buf, MAXDATASIZE, 0);

    int fd2 = open(path, O_RDONLY);
    if (fd2 < 0)        //打开文件失败
    {
        perror("open");
        exit(-3);
    }

    while (1)
    {
        int len = read(fd2,buf,sizeof(buf));
        if (len == 0)
            break;    
        int _tmp = 0;
        while (1)
        {
            int ret = send(server_sockfd, buf + _tmp, len - _tmp, 0);
            if (ret > 0)
                _tmp += ret;
            if (_tmp == ret)
                break;
            if (ret < 0)
            {
                perror("write");
                break;
            }
        }
    }
    printf("file is uploaded\n");
    return ;
}

void send_data(int fd){  
    int server_sockfd = fd;
    char buf[MAXDATASIZE];
    memset(buf, 0, sizeof(buf));
    while(1){

        printf("input:");
        fgets(buf, MAXDATASIZE, stdin);
        if(buf[strlen(buf)-1] == '\n')
            buf[strlen(buf)-1] = '\0';
        fflush(stdin);                     //清除输入缓存 
        
        if(strncmp(buf, "/exit", strlen("/exit")) == 0){
            if(send(server_sockfd, buf, sizeof(buf), 0) == -1){
                perror("send error");
                exit(EXIT_FAILURE);
            }
            break;
        }
        else if(strncmp(buf, "/file", strlen("/file")) == 0)
        {
            if(send(server_sockfd, buf, sizeof(buf), 0) == -1)
            {
                perror("send error");
                exit(EXIT_FAILURE);
            }
            send_file(fd);
            continue;
        }
        if(send(server_sockfd, buf, sizeof(buf), 0) == -1){
             perror("send error");
             exit(EXIT_FAILURE);
        }
    }
    printf("client will be closed, see you next time.\n");
    close(server_sockfd);
    exit(0);
 
}
int main(int argc, char *argv[])
{
    if(argc != 2){
        fprintf(stderr, "Usage: ./client <IP> \n");
        exit(EXIT_FAILURE);
    }

    char recv_buf[MAXDATASIZE] = {'\0'}, send_buf[MAXDATASIZE] = {'\0'};
    pthread_t id;
    int sockfd;
    const char *server_ip = argv[1]; //从命令行获取输入的ip地址
    struct sockaddr_in serveraddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &serveraddr.sin_addr);
    connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
   
    recv(sockfd, recv_buf, MAXDATASIZE/sizeof (char), 0);
    if (strncmp(recv_buf, "WELCOME!", strlen("WELCOME!")) == 0)
    	printf("=====================服务器链接成功=====================\n");
    else if (strncmp(recv_buf, "FULL!", strlen("FULL!")) == 0)
    {
        printf("=====================服务器已满！！=====================\n");
        exit(EXIT_FAILURE);
    }
    else
    {
	    printf("=====================服务器链接失败=====================\n");
	    exit(EXIT_FAILURE);
    }
    fputs(recv_buf,stdout);
    while(1)
    {
        recv(sockfd, recv_buf, MAXDATASIZE/sizeof (char), 0);
        fputs(recv_buf,stdout);
        fgets(send_buf, sizeof(send_buf), stdin);
        fflush(stdin);
        send(sockfd, send_buf, sizeof(send_buf), 0);
        recv(sockfd, recv_buf, MAXDATASIZE/sizeof (char), 0);
        if(strncmp(recv_buf, "ok", strlen("ok")) == 0)
            break;
        else
            fputs("This name has been used!\n", stdout);
            
    }

    if(pthread_create(&id, NULL, recv_data, &sockfd)!=0){         //创建子线程 
            perror("pthread_create");
        }
  
    send_data(sockfd);
  return 0;
}

/**
  * @brief 接受数据的线程
  * @param 服务器端socket文件描述符
  * @retval None
  * @details None
  */
void *recv_data(void *fd)
{
   char recv_buf[MAXDATASIZE] = {'\0'};
   int server_fd = *(int *)fd;
   while(1){
       recv(server_fd, recv_buf, MAXDATASIZE/sizeof (char), 0);
       fputs("\n", stdout);
       fputs(recv_buf,stdout);
       fputs("\n", stdout);
       fflush(stdout);
   }
   pthread_exit(NULL);
}

