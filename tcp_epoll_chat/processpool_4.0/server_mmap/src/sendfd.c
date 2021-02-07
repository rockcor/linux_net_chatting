#include "process_pool.h"

//通过管道把描述符传给另外一个进程
int sendFd(int pipefd,int fd,char exitFlag)
{
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));

    struct iovec iov;
    memset(&iov,0,sizeof(iov));
    iov.iov_base=&exitFlag;
    iov.iov_len=sizeof(exitFlag);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    int len=CMSG_LEN(sizeof(int));
    struct cmsghdr *cmsg=(struct cmsghdr*)calloc(1,len);

    cmsg->cmsg_len=len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type=SCM_RIGHTS;
    *(int*)CMSG_DATA(cmsg)=fd;

    msg.msg_control=cmsg;
    msg.msg_controllen=len;

    int ret=sendmsg(pipefd,&msg,0);
    ERROR_CHECK(ret,-1,"sendmsg");

    free(cmsg);
    cmsg=NULL;

    return 0;
}

//从管道接收描述符
int recvFd(int pipefd,int *fd,char* exitFlag)
{
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));

    struct iovec iov;
    memset(&iov,0,sizeof(iov));
    iov.iov_base=exitFlag;
    iov.iov_len=sizeof(char);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    int len=CMSG_LEN(sizeof(int));
    struct cmsghdr *cmsg=(struct cmsghdr*)calloc(1,len);

    cmsg->cmsg_len=len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type=SCM_RIGHTS;

    msg.msg_control=cmsg;
    msg.msg_controllen=len;

    int ret=recvmsg(pipefd,&msg,0);
    ERROR_CHECK(ret,-1,"sendmsg");

    *fd=*(int*)CMSG_DATA(cmsg);

    free(cmsg);
    cmsg=NULL;

    return 0;
}
