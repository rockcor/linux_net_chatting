#include <func.h>

#define MAXFD 10

typedef struct
{
    char name[64];
    char msg[128];
}Messege;

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);

    //创建socket            ipv4    tcp
    int serverFd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(serverFd,-1,"socket");

    //记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;//ipv4
    serAddr.sin_addr.s_addr=inet_addr(argv[1]);//ip格式转化
    serAddr.sin_port=htons(atoi(argv[2]));//设置端口为网络字节序
    
    //connect连接服务器
    int ret=connect(serverFd,(struct sockaddr*)&serAddr,
                    sizeof(serAddr));
    ERROR_CHECK(ret,-1,"connect");

    Messege msg;
    memset(&msg,0,sizeof(msg));

    fd_set rdset;
    FD_ZERO(&rdset);

    //select的返回值是可读文件描述符的数量
    int readyNum =0;

    while(1)
    {
        FD_SET(STDIN_FILENO,&rdset); 
        FD_SET(serverFd,&rdset); 
        readyNum = select(MAXFD,&rdset,NULL,NULL,NULL);

        if(readyNum>0)
        {
            //向服务器发送数据
            if(FD_ISSET(STDIN_FILENO,&rdset))
            {
                memset(&msg,0,sizeof(msg));
                read(STDIN_FILENO,msg.msg,sizeof(msg.msg));
                send(serverFd,&msg,sizeof(msg),0);
            }

            //接收服务器的数据
            if(FD_ISSET(serverFd,&rdset)){
                memset(&msg,0,sizeof(msg));
                ret = recv(serverFd,&msg,sizeof(msg),0);
                if(0 == ret)
                {
                    printf("服务器断开连接\n");
                    break;
                }
                printf("%s:\n%s",msg.name,msg.msg);
            }
        }
    }

    close(serverFd);
    return 0;
}

