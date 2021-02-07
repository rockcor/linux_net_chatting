#include "process_pool.h"

int epollAdd(int epfd,int fd)
{
    struct epoll_event evt;
    memset(&evt,0,sizeof(evt));

    evt.data.fd=fd;
    evt.events=EPOLLIN;

    int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&evt);
    ERROR_CHECK(ret,-1,"EPOLL_CTL_ADD");
    return 0;
}

