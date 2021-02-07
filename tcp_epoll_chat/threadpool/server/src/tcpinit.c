#include "../include/threadpool.h"

int tcpInit(int *socketFd,char *ip,char *port)
{
    //创建监听socket
    int listenFd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(listenFd,-1,"socket");

    //设置地址可重用
    int reuse=1;
    int ret=0;
    ret=setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&reuse,
                   sizeof(reuse));
    ERROR_CHECK(ret,-1,"setsockopt");

    //记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;
    serAddr.sin_addr.s_addr=inet_addr(ip);
    serAddr.sin_port=htons(atoi(port));

    //绑定ip和端口
    ret=bind(listenFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"bind");

    //建立监听
    ret=listen(listenFd,10);
    ERROR_CHECK(ret,-1,"listen");

    *socketFd=listenFd;

    return 0;
}

