/*************************************************************************
	> File Name: server.c
	> Author: Name
	> Mail: Name@163.com 
	> Created Time: 2021-01-29 17:25:48
 ************************************************************************/

#include <head.h>

#define MAXFD 10

int main(int argc,char* argv[])
{
    ARGS_CHECK(argc,3);

    //先创建监听套接字,用于接收服务端的连接请求
    int listenFd= socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(listenFd,-1,"socket");

    //设置地址可重用，允许绑定TIME_WAIT状态的地址
    //
    int reuse = 1;
    int ret = 0;
    ret = setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ERROR_CHECK(ret,-1,"setsockopt");

    //定义结构体，记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    //把字符串的点分十进制ip转换成网络字节序的二进制ip
    serAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serAddr.sin_port = htons(atoi(argv[2]));

    //绑定ip和端口号
    ret = bind(listenFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"bind");

    //建立监听，等待客户端连接
    //10表示全连接队列的大小
    ret = listen(listenFd,10); 
    ERROR_CHECK(ret,-1,"listen");

    //服务器等待客户端连接，如果没有客户端连接
    //服务器程序会阻塞在accept函数这里
    //accpet返回一个新的套接字，
    //这个套接字代表客户端和服务器的TCP连接
    //后面通过clientFd套接字去跟客户端通信
    int clientFd=0;
    clientFd = accept(listenFd,NULL,NULL);
    ERROR_CHECK(clientFd,-1,"accept");

    //完成三次握手，可以正常通信
    //send和recv的用法类似write和read
    char buf[64]={0};

    //rdset只负责给select传参
    fd_set rdset;
    FD_ZERO(&rdset);

    fd_set needMoniterSet;
    FD_ZERO(&needMoniterSet);
    //needMoniterSet记录的是需要让select监听的描述符
    FD_SET(STDIN_FILENO,&needMoniterSet);
    FD_SET(listenFd,&needMoniterSet);
    FD_SET(clientFd,&needMoniterSet);


    //select的返回值是可读文件描述符的数量
    int readyNum =0;

    while(1)
    {
        FD_ZERO(&rdset);
        //把需要监听的描述符拷贝给rdset,由rdset给select传参
        //这样needMoniterSet中的描述符不变
        memcpy(&rdset,&needMoniterSet,sizeof(needMoniterSet));
        //当有客户端连接的时候，listenFd变成就绪
        //select会返回，并且返回时的rdset中保存着listenFd
        readyNum = select(MAXFD,&rdset,NULL,NULL,NULL);
        ERROR_CHECK(readyNum,-1,"select");
        if(readyNum>0)
        {
            if(FD_ISSET(STDIN_FILENO,&rdset))
            {
                memset(buf,0,sizeof(buf));
                read(STDIN_FILENO,buf,sizeof(buf));
                //-1是为了不把最后的\n发出去
                send(clientFd,buf,strlen(buf)-1,0);
            }
            if(FD_ISSET(clientFd,&rdset)){
                //clientFd可读，表示客户端有数据到达
                memset(buf,0,sizeof(buf));
                //对端断开时，recv的返回值是0
                ret = recv(clientFd,buf,sizeof(buf),0);
                if(0 == ret)
                {
                    printf("byebye\n");
                    //对端断开，关闭连接
                    //close关闭连接之后，clientFd已经不再是描述符了
                    //clientFd只是单纯的数字
                    close(clientFd);
                    //客户端断开，此时从needMoniterSet里删除
                    //不需要再监听了
                    FD_CLR(clientFd,&needMoniterSet);
                }
                else
                    printf("buf=%s\n",buf);
            }
            if(FD_ISSET(listenFd,&rdset))
            {
                //如果FD_ISSET成立，表示客户端连接
                clientFd = accept(listenFd,NULL,NULL);
                /* FD_SET(clientFd,&rdset); */
                printf("clientFd = %d\n",clientFd);
                //如果客户端又连接上来，重新加入到需要监听的集合中
                //继续让select监听
                FD_SET(clientFd,&needMoniterSet);
            }
        }
    }

    //clientFd和listenFd，都是文件描述符
    //通过close关闭即可
    //可以自己打印一下描述符的值
    close(listenFd);
    close(clientFd);
    return 0;
}
