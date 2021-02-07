#include "../include/head.h"
#include "../include/threadpool.h"

int fds[2];

//收到信号后让线程池退出
void exitFunc(int signum)
{
    char exitFlag=1;
    write(fds[1],&exitFlag,sizeof(exitFlag));
    printf("signal %d is coming\n",signum);
}

//ip、端口、线程数量
int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,4);

    //创建退出管道
    pipe(fds);

    //捕捉信号
    signal(SIGUSR1,exitFunc);

    //子线程个数
    int threadNum=atoi(argv[3]);
    threadPool_t threadPool;
    memset(&threadPool,0,sizeof(threadPool));

    //1.初始化线程池
    threadPoolInit(&threadPool,threadNum);

    //2.启动线程池
    threadPoolStart(&threadPool);

    //3.建立tcp监听
    int listenFd=0;
    tcpInit(&listenFd,argv[1],argv[2]);

    //4.epoll
    int epfd =epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create");

    epollAdd(epfd,listenFd);
    epollAdd(epfd,fds[0]);

    int readyNum=0;
    int clientFd=0;
    struct epoll_event evs[2];
    memset(evs,0,sizeof(evs));

    while(1)
    {
        readyNum=epoll_wait(epfd,evs,2,-1);
        for(int i=0;i<readyNum;i++)
        {
            if(evs[i].data.fd==listenFd)
            {
                //接收客户端连接
                clientFd=accept(listenFd,NULL,NULL);
                ERROR_CHECK(clientFd,-1,"accept");

                //插入队列
                pNode_t pNew=(pNode_t)calloc(1,sizeof(node_t));
                pNew->clientFd=clientFd;
                pthread_mutex_lock(&threadPool.que.mLock);
                queInsert(&threadPool.que,pNew);
                pthread_mutex_unlock(&threadPool.que.mLock);

                //通知子线程
                pthread_cond_signal(&threadPool.que.cond);
            }
            else if(evs[i].data.fd==fds[0])//让子线程有序退出
            {
                printf("pipe\n");
                threadPool.stat=0;
                for(int j=0;j<threadNum;j++)
                {
                    pthread_cond_signal(&threadPool.que.cond);
                    pthread_join(threadPool.pThid[j],NULL);
                }
                printf("parent exit\n");
                exit(0);
            }
        }
    }

    close(listenFd);

    return 0;
}
