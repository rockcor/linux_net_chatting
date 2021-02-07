#include "process_pool.h"

int makeChild(int processNum,pProcessData_t pData)
{
    //循环创建子进程，建立管道
    //记录子进程信息
    pid_t pid;
    int fds[2];
    int ret=0;

    for(int i=0;i<processNum;i++)
    {
        //创建管道
        ret=socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
        ERROR_CHECK(ret,-1,"socketpair");
        
        pid=fork();
        if(0==pid)//子进程
        {
            close(fds[0]);
            childFunc(fds[1]);//子进程用fds1
            exit(0);
        }
        close(fds[1]);

        //记录子进程信息
        pData[i].busy=0;
        pData[i].pid=pid;
        pData[i].pipefd=fds[0];//父进程用fds0
    }

    return 0;
}

int childFunc(int pipefd)
{
    int clientFd;
    char notBusy=0;
    char exitFlag=0;

    while(1)
    {
        //接收父进程发来的clientFd
        recvFd(pipefd,&clientFd,&exitFlag);

        if(1==exitFlag)
        {
            printf("child exit\n");
            close(pipefd);
            exit(0);
        }

        //发送文件
        transFile(clientFd);

        printf("byebye client %d\n",clientFd);
        close(clientFd);

        //通知父进程已完成
        write(pipefd,&notBusy,1);
    }
}
