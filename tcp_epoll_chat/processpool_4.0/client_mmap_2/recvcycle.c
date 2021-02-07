#include <func.h>

int recvCycle(int socketFd,void *buf,size_t totallen)
{
    size_t recvLen=0;//已接收字节
    ssize_t ret=0;//接收的字节

    while(recvLen<totallen)
    {
        ret=recv(socketFd,(char*)buf+recvLen,totallen-recvLen,0);
        recvLen+=ret;
    }

    return 0;
}

