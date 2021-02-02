#include "func.h"

#define MAXFD 20

int setNonblock(int fd)
{
    //保存文件描述符当前的状态
    int status=0;
    status = fcntl(fd,F_GETFL);
    ERROR_CHECK(status,-1,"F_GETFL");
    status = status|O_NONBLOCK;
    int ret = fcntl(fd,F_SETFL,status);
    ERROR_CHECK(ret,-1,"F_SETFL");

    return 0;
}
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
    char buf[4];
}Message;

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);//./server own_ip specified_port

    //创建socket            ipv4    udp
    int serverFd=socket(AF_INET,SOCK_DGRAM,0);
    ERROR_CHECK(serverFd,-1,"socket");

    //设置地址可重用，允许绑定TIME_WAIT状态的地址
    int reuse=1;
    int ret=0;
    ret=setsockopt(serverFd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ERROR_CHECK(ret,-1,"setsockopt");

    //记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;//ipv4
    serAddr.sin_addr.s_addr=inet_addr(argv[1]);//ip点分十进制转网络字节序
    serAddr.sin_port=htons(atoi(argv[2]));//设置端口为网络字节序

    //bind绑定ip和端口     把新类型转成老类型
    ret=bind(serverFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"bind");

    Client client[10];//定义结构体数组，存储连接上的客户端fd等
    memset(&client,0,sizeof(client));
    //不用socklen_t会报错
    socklen_t addrlen=sizeof(client[0].clientAddr);

    Message msg;
    memset(&msg,0,sizeof(msg));

    setNonblock(serverFd);
    int readyNum =0;
    int epfd = 0;
    //epoll_create创建一个epoll实例，返回值是一个文件描述符
    //后续对epoll的操作，都通过描述符进行
    //epoll_create的传参填的是一个非0的正数，已经没什么含义了
    epfd = epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create");

    //epoll使用的第二步，需要把我们关心的描述符
    //交给epoll管理
    struct epoll_event evt;
    memset(&evt,0,sizeof(evt));
    //填充结构体，填充事件类型，EPOLLIN表示的描述符上面有读事件
    //表示描述符可读
    //还要填充我们关心的是哪个描述符
    evt.events = EPOLLIN;
    evt.data.fd = STDIN_FILENO;
    ret = epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&evt);
    ERROR_CHECK(ret,-1,"epoll_ctl");

    //设置为边沿触发模式
    evt.events = EPOLLIN|EPOLLET;
    evt.data.fd = serverFd;
    ret = epoll_ctl(epfd,EPOLL_CTL_ADD,serverFd,&evt);
    ERROR_CHECK(ret,-1,"epoll_ctl");

    struct epoll_event *evs = (struct epoll_event*)calloc(2,sizeof(struct epoll_event));

    //可读文件描述符的数量
    //随便用一个缓冲区接一下，主要为了获得clientaddr
    while(1)
    {
        readyNum=epoll_wait(epfd,evs,2,-1);
        ERROR_CHECK(readyNum,-1,"epoll_wait");
        if(readyNum>0)
        {
            for(int i=0;i<readyNum;i++)
            {
                if(evs[i].data.fd==serverFd){
                    while(1)
                    {
                        {
                            memset(&msg,0,sizeof(msg));
                            ret = recvfrom(serverFd,msg.buf,sizeof(msg.buf),0,(struct sockaddr*)&client[0].clientAddr,&addrlen);
                            /* strcpy(msg.name,inet_ntoa(client[0].clientAddr.sin_addr)); */
                            if(0==ret)
                            {
                                printf("断开连接\n");
                                ret = epoll_ctl(epfd,EPOLL_CTL_DEL,serverFd,NULL);
                                ERROR_CHECK(ret,-1,"epoll_ctl_del");
                                close(serverFd);
                                break;
                            }else if(-1==ret)
                            {
                                break;
                            }
                            msg.buf[3]=0;
                            printf("%s",msg.buf);
                        }
                    }
                } else if(evs[i].data.fd==STDIN_FILENO)
                    {
                        memset(&msg,0,sizeof(msg));
                        read(STDIN_FILENO,msg.buf,sizeof(msg.buf)-1);
                        /* strcpy(msg.name,"server"); */
                        sendto(serverFd,msg.buf,strlen(msg.buf),0,(struct sockaddr*)&client[0].clientAddr,addrlen);
                    }

                       
            }

        }
    }
    close(serverFd);
    return 0;
}
