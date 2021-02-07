#include "process_pool.h"

int fds[2];

//收到信号后让进程池退出
void exitFunc(int signum)
{
    char exitFlag=1;
    write(fds[0],&exitFlag,sizeof(exitFlag));
    printf("signal %d is coming\n",signum);
}

//ip、端口、进程数量
int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,4);

    //创建退出管道
    pipe(fds);

    //1.创建子进程，建立管道，保存子进程信息
    //子进程数
    int processNum=atoi(argv[3]);

    //创建结构体数组，保存子进程信息
    pProcessData_t pData=(pProcessData_t)calloc(processNum,
                                                sizeof(ProcessData_t));
    makeChild(processNum,pData);

    //创建子进程后再设置捕捉信号
    signal(SIGUSR1,exitFunc);

    //2.建立TCP监听
    int listenFd=0;
    tcpInit(&listenFd,argv[1],argv[2]);

    //3.epoll
    //创建epoll
    int epfd=epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create");

    //添加监听socket到epoll里
    epollAdd(epfd,listenFd);

    //添加父子进程通信的管道到epoll里
    for(int i=0;i<processNum;i++)
    {
        epollAdd(epfd,pData[i].pipefd);
    }

    //4.
    int clientFd=0;
    int readyNum=0;
    int i=0;
    char busy=0;//busy,notBusy:提高程序可读性
    char exitFlag=0;//退出标志

    struct epoll_event *evs=
        (struct epoll_event*)calloc(processNum,sizeof(struct epoll_event));
    
    while(1)
    {
        readyNum=epoll_wait(epfd,evs,processNum,-1);
        
        for(i=0;i<readyNum;i++)
        {
            //有新客户端连接
            if(evs[i].data.fd==listenFd)
            {
                //获取clienFd
                clientFd=accept(listenFd,NULL,NULL);
                //找到空闲子进程，把客户端socket传递给子进程
                for(int j=0;j<processNum;j++)
                {
                    if(0==pData[j].busy)
                    {
                        exitFlag=0;
                        sendFd(pData[j].pipefd,clientFd,exitFlag);
                        pData[j].busy=1;
                        printf("child %d is busy\n",pData[j].pid);
                        break;
                    }
                }

                //1.把clientFd传递给子进程后，clientFd引用计数为2，
                //父进程close后引用计数为1，子进程close后TCP才会断
                //开
                //2.所有客户端都忙时，直接close
                close(clientFd);
            }
            else if(evs[i].data.fd==fds[1])//让子进程有序退出
            {
                for(int j=0;j<processNum;j++)
                {
                    exitFlag=1;
                    sendFd(pData[j].pipefd,0,exitFlag);
                }

                //父进程等待子进程退出完毕后退出
                for(int j=0;j<processNum;j++)
                {
                    wait(NULL);
                }

                printf("parent exit\n");
                exit(0);
            }
            else//找到可读的管道,也就是完成任务的子进程
            {
                for(int j=0;j<processNum;j++)
                {
                    if(evs[j].data.fd==pData[j].pipefd)
                    {
                        pData[j].busy=0;
                        read(pData[j].pipefd,&busy,sizeof(busy));
                        printf("child %d is not busy\n",pData[j].pid);
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

