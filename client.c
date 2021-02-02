#include "func.h"

#define MAXFD 5

typedef struct
{
    char name[64];
    char buf[4];
}Messege;

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

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);
    //创建socket            ipv4    udp
    int serverFd=socket(AF_INET,SOCK_DGRAM,0);
    ERROR_CHECK(serverFd,-1,"socket");

    //记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;//ipv4
    serAddr.sin_addr.s_addr=inet_addr(argv[1]);//ip格式转化
    serAddr.sin_port=htons(atoi(argv[2]));//设置端口为网络字节序
    socklen_t len=sizeof(serAddr);
    int ret=0;

    Messege msg;
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


    while(1)
    {
        readyNum=epoll_wait(epfd,evs,2,-1);
        ERROR_CHECK(readyNum,-1,"epoll_wait");
        if(readyNum>0)
        {
            for(int i=0;i<readyNum;i++)
            {
                //向服务器发送数据
                if(evs[i].data.fd==STDIN_FILENO)
                {
                    memset(&msg,0,sizeof(msg));
                    read(STDIN_FILENO,msg.buf,sizeof(msg.buf)-1);
                    sendto(serverFd,msg.buf,strlen(msg.buf),0,(struct sockaddr*)&serAddr,len);
                }

                //接收服务器的数据
                else if(evs[i].data.fd==serverFd){
                    while(1)
                    {
                        {
                            memset(&msg,0,sizeof(msg));
                            ret = recvfrom(serverFd,msg.buf,sizeof(msg.buf),0,(struct sockaddr*)&serAddr,&len);
                            if(0==ret)
                            {
                                printf("服务器断开连接\n");
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
                }
            }
        }
    }

    close(serverFd);
    return 0;
}

