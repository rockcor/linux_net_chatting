#include "../include/threadpool.h"

void* threadFunc(void* pArg)
{
    pThreadPool_t pPool=(pThreadPool_t)pArg;
    pQue_t pQue=&pPool->que;
    pNode_t pCur=NULL;
    int ret=0;

    while(1)
    {
        if(0==pPool->stat)
        {
            printf("child exit\n");
            pthread_exit(NULL);
        }
        //判断队列是否为空
        pthread_mutex_lock(&pQue->mLock);
        if(0==pQue->size)
        {
            printf("thread wait\n");
            pthread_cond_wait(&pQue->cond,&pQue->mLock);
            printf("thread wake up\n");
        }
        //获取clientfd结点
        ret=queGet(pQue,&pCur);
        pthread_mutex_unlock(&pQue->mLock);

        //传输文件
        if(-1!=ret)
        {
            transFile(pCur->clientFd);
            printf("finish\n");
            close(pCur->clientFd);
        }
    }
}

int threadPoolInit(pThreadPool_t pPool,int threadNum)
{
    memset(pPool,0,sizeof(threadPool_t));

    //赋初值
    queInit(&pPool->que);
    pPool->threadNum=threadNum;
    pPool->stat=0;

    //保存线程id的数组
    pPool->pThid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));

    return 0;
}

int threadPoolStart(pThreadPool_t pPool)
{
    //若线程池未启动
    if(0==pPool->stat)
    {
        pPool->stat=1;
        //创建子线程
        for(int i=0;i<pPool->threadNum;i++)
        {
            pthread_create(pPool->pThid+i,NULL,threadFunc,pPool);
        }
    }

    return 0;
}
