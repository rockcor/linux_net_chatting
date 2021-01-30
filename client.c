/*************************************************************************
	> File Name: client.c
	> Author: Name
	> Mail: Name@163.com 
	> Created Time: 2021-01-29 17:54:15
 ************************************************************************/

#define MAXFD 10
#include <head.h>

int main(int argc,char* argv[])
{
    ARGS_CHECK(argc,3);

    //先创建套接字,用于跟服务端通信
    int serverFd = socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(serverFd,-1,"socket");

    //定义结构体，记录ip和端口
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    //把字符串的点分十进制ip转换成网络字节序的二进制ip
    serAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serAddr.sin_port = htons(atoi(argv[2]));
 
    //connect连接服务器，结构体中填写的是服务器的ip和端口
    int ret = connect(serverFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"connect");


    char buf[64]={0};

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
            if(FD_ISSET(STDIN_FILENO,&rdset))
            {
                memset(buf,0,sizeof(buf));
                read(STDIN_FILENO,buf,sizeof(buf));
                //-1是为了不把最后的\n发出去
                send(serverFd,buf,strlen(buf)-1,0);
            }
            if(FD_ISSET(serverFd,&rdset)){
                //serverFd可读，表示客户端有数据到达
                memset(buf,0,sizeof(buf));
                ret = recv(serverFd,buf,sizeof(buf),0);
                if(0 == ret)
                {
                    printf("byebye\n");
                    break;
                }
                printf("buf=%s\n",buf);
            }
        }
    }

    close(serverFd);
    return 0;
}
