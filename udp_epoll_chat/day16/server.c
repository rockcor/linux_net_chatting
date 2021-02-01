#include <func.h>

#define MAXFD 20

typedef struct
{
    struct sockaddr_in cliAddr;
    int stat;//连接状态，0未连接，1已连接
}Client;

//发送的消息
typedef struct
{
    char name[64];
    char msg[128];
}Messege;

//保存文件描述符当前的状态
int setNonblock(int fd)
{
    int status=0;
    status = fcntl(fd,F_GETFL);
    ERROR_CHECK(status,-1,"F_GETFL");
    status = status|O_NONBLOCK;
    int ret = fcntl(fd,F_SETFL,status);
    ERROR_CHECK(ret,-1,"F_SETFL");
    return 0;
}

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);

    //创建socket            ipv4    udp
    int serverFd=socket(AF_INET,SOCK_DGRAM,0);
    ERROR_CHECK(serverFd,-1,"socket");

    //设置地址可重用，允许绑定TIME_WAIT状态的地址
    int reuse=1;
    int ret=0;
    ret=setsockopt(serverFd,SOL_SOCKET,SO_REUSEADDR,&reuse,
                   sizeof(reuse));
    ERROR_CHECK(ret,-1,"setsockopt");

    //设置非阻塞
    setNonblock(serverFd);

    //记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;//ipv4
    serAddr.sin_addr.s_addr=inet_addr(argv[1]);//ip格式转化
    serAddr.sin_port=htons(atoi(argv[2]));//设置端口为网络字节序
    
    //bind绑定ip和端口     把新类型转成老类型
    ret=bind(serverFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"bind");

    //客户端ip和端口
    Client client[10];
    memset(&client,0,sizeof(client));
    socklen_t len[10];
    for(int i=0;i<10;i++)
    {
        len[i]=sizeof(client[i].cliAddr);
    }

    Messege msg;
    memset(&msg,0,sizeof(msg));

    //创建一个epoll实例，参数为非0正数，无含义
    int epfd=epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create");

    //把描述符交给epoll管理
    struct epoll_event evt,cliEvt[20];
    memset(&evt,0,sizeof(evt));

    evt.events=EPOLLIN|EPOLLET;
    evt.data.fd=serverFd;
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,serverFd,&evt);
    ERROR_CHECK(ret,-1,"epoll_ctl");

    int readyNum=0;

    struct sockaddr_in tmpAddr;
    memset(&tmpAddr,0,sizeof(tmpAddr));
    socklen_t tmpLen=sizeof(tmpAddr);

    while(1)
    {
        readyNum=epoll_wait(epfd,cliEvt,20,-1);
        ERROR_CHECK(readyNum,-1,"epoll_wait");
        if(readyNum>0)
        {
            for(int i=0;i<readyNum;i++)
            {
                //有新客户端连接，或有消息发过来
                if(cliEvt[i].data.fd==serverFd)
                {
                    int j;
                    memset(&msg,0,sizeof(msg));
                    ret=recvfrom(serverFd,&msg,sizeof(msg),0,
                                 (struct sockaddr*)&tmpAddr,
                                 &tmpLen);
                    ERROR_CHECK(ret,-1,"recvfrom");
                    for(j=0;j<10;j++)
                    {
                        //ip和端口相同，则发送信息
                        if(client[j].cliAddr.sin_addr.s_addr==tmpAddr.sin_addr.s_addr&&client[j].cliAddr.sin_port==tmpAddr.sin_port)
                        {
                            if(0==ret)//客户端断开连接
                            {
                                memset(&client[j],0,sizeof(client[j]));
                                printf("客户端断开连接\n");
                                break;
                            }
                            for(int k=0;k<10;k++)
                            {
                                if(k==j)
                                {
                                    continue;
                                }
                                sendto(serverFd,&msg,sizeof(msg),0,(struct sockaddr*)&client[k].cliAddr,len[k]);
                            }
                            break;
                        }
                    }

                    if(j==10)//无相同ip和端口，新连接
                    {
                        for(int k=0;k<10;k++)
                        {
                            if(0==client[k].stat)
                            {
                                memcpy(&client[k].cliAddr,&tmpAddr,sizeof(tmpAddr));
                                client[k].stat=1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}

