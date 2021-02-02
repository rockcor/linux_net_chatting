#include <func.h>

#define MAXFD 10

int serverFd;
struct sockaddr_in serAddr;
socklen_t len;

typedef struct
{
    char ip[64];
    uint16_t port;
    char msg[128];
    short link;
}Messege;

void sigFunc(int signum)
{
    Messege msg;
    memset(&msg,0,sizeof(msg));

    sendto(serverFd,&msg,sizeof(msg),0,(struct sockaddr*)&serAddr,len);
}

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);

    //创建socket            ipv4    tcp
    serverFd=socket(AF_INET,SOCK_DGRAM,0);
    ERROR_CHECK(serverFd,-1,"socket");

    //记录ip和端口
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;//ipv4
    serAddr.sin_addr.s_addr=inet_addr(argv[1]);//ip格式转化
    serAddr.sin_port=htons(atoi(argv[2]));//设置端口为网络字节序
    
    len=sizeof(serAddr);

    Messege msg;
    memset(&msg,0,sizeof(msg));

    msg.link=1;
    int ret=sendto(serverFd,&msg,sizeof(msg),0,(struct sockaddr*)&serAddr,len);
    ERROR_CHECK(ret,-1,"sendto");

    //创建一个epoll实例，参数为非0正数，无含义
    int epfd=epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create");

    //把描述符交给epoll管理
    struct epoll_event evt,cliEvt[3];
    memset(&evt,0,sizeof(evt));

    //添加serverFD
    evt.events=EPOLLIN|EPOLLET;
    evt.data.fd=serverFd;
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,serverFd,&evt);
    ERROR_CHECK(ret,-1,"epoll_ctl");

    //添加标准输入
    evt.events = EPOLLIN;
    evt.data.fd = STDIN_FILENO;
    ret = epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&evt);
    ERROR_CHECK(ret,-1,"epoll_ctl");
    int readyNum =0;

    signal(SIGINT,sigFunc);

    while(1)
    {
        readyNum=epoll_wait(epfd,cliEvt,3,-1);
        ERROR_CHECK(readyNum,-1,"epoll_wait");

        if(readyNum>0)
        {
            for(int i=0;i<readyNum;i++)
            {
                //服务器发消息过来
                if(cliEvt[i].data.fd==serverFd)
                {
                    memset(&msg,0,sizeof(msg));
                    ret=recvfrom(serverFd,&msg,sizeof(msg),0,
                                 (struct sockaddr*)&serAddr,&len);
                    ERROR_CHECK(ret,-1,"recvfrom");
                    printf("%s %d:\n%s",msg.ip,msg.port,msg.msg);
                }
                //发消息给服务器
                if(cliEvt[i].data.fd==STDIN_FILENO)
                {
                    memset(&msg,0,sizeof(msg));
                    msg.link=1;
                    read(STDIN_FILENO,msg.msg,sizeof(msg.msg));
                    int ret=sendto(serverFd,&msg,sizeof(msg),0,(struct sockaddr*)&serAddr,len);
                    ERROR_CHECK(ret,-1,"sendto");
                }
            }
        }
    }

    close(serverFd);
    return 0;
}

