#include "func.h"

#define MAXFD 20

typedef struct
{
    int clientFd;
    struct sockaddr_in clientAddr;
    int stat;//连接状态，0未连接，1已连接
}Client;

//发送的消息
typedef struct
{
    char name[64];
    char buf[128];
}Message;

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);//./server own_ip specified_port

    //创建socket            ipv4    tcp
    int listenFd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(listenFd,-1,"socket");

    //设置地址可重用，允许绑定TIME_WAIT状态的地址
    int reuse=1;
    int ret=0;
    ret=setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ERROR_CHECK(ret,-1,"setsockopt");

    //记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;//ipv4
    serAddr.sin_addr.s_addr=inet_addr(argv[1]);//ip点分十进制转网络字节序
    serAddr.sin_port=htons(atoi(argv[2]));//设置端口为网络字节序

    //bind绑定ip和端口     把新类型转成老类型
    ret=bind(listenFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"bind");

    //listen监听        指定同时能处理的最大连接要求，也是全连接套接字队列大小，最大128
    ret=listen(listenFd,20);
    ERROR_CHECK(ret,-1,"listen");

    Client client[10];//定义结构体数组，存储连接上的客户端fd等
    memset(&client,0,sizeof(client));
    //不用socklen_t会报错
    socklen_t addrlen=sizeof(client[0].clientAddr);

    Message msg;
    memset(&msg,0,sizeof(msg));

    fd_set rdset;

    fd_set moniterSet;
    FD_ZERO(&moniterSet);
    FD_SET(listenFd,&moniterSet);

    //可读文件描述符的数量
    int readyNum=0;

    while(1)
    {
        FD_ZERO(&rdset);
        memcpy(&rdset,&moniterSet,sizeof(moniterSet));
        readyNum=select(MAXFD,&rdset,NULL,NULL,NULL);
        ERROR_CHECK(readyNum,-1,"select");

        if(readyNum>0)
        {
            //accept
            if(FD_ISSET(listenFd,&rdset))
            {
                for(int j=0;j<10;j++)
                {
                    //找到第一个空缺位,保持和FD逻辑一致，客户端号等于始终等于下标+4
                    //也可以采用循环队列的策略
                    if(0==client[j].stat)
                    {
                        client[j].clientFd=accept(listenFd,(struct sockaddr*)&client[j].clientAddr,&addrlen) ;
                        client[j].stat=1;
                        printf("%s已连接\n", inet_ntoa(client[j].clientAddr.sin_addr));
                        FD_SET(client[j].clientFd,&moniterSet);
                        break;
                    }
                }
            }

            //接收客户端发来的数据，并发送给其他客户端
            for(int i=0;i<10;i++)
            {
                if(FD_ISSET(client[i].clientFd,&rdset))
                {
                    memset(&msg,0,sizeof(msg));
                    ret=recv(client[i].clientFd,&msg,sizeof(msg),0);
                    if(0==ret)
                    {
                        printf("%s断开连接\n",inet_ntoa(client[i].clientAddr.sin_addr));
                        close(client[i].clientFd);
                        FD_CLR(client[i].clientFd,&moniterSet);
                        memset(&client[i],0,sizeof(client[i]));
                    }
                    else
                    {
                        strcpy(msg.name,inet_ntoa(client[i].clientAddr.sin_addr));
                        for(int j=0;j<10;j++)
                        {
                            //发给其他所有连接着的客户端
                            if(j!=i&&1==client[j].stat)
                                send(client[j].clientFd,&msg,sizeof(msg),0);
                        }
                    }
                }
            }
        }
    }

    close(listenFd);
    return 0;
}

