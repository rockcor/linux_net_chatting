#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "head.h"
#include "workque.h"

//线程池
typedef struct
{
    que_t que;
    int threadNum;
    pthread_t *pThid;//保存线程id的数组
    short stat;
}threadPool_t,*pThreadPool_t;

//小火车
typedef struct
{
    int dataLen;
    char buf[1000];
}train_t;

int threadPoolInit(pThreadPool_t,int);
int threadPoolStart(pThreadPool_t);

int tcpInit(int*,char*,char*);
int epollAdd(int,int);
int transFile(int);

#endif
