#ifndef __WORKQUE_H__
#define __WORKQUE_H__

#include "head.h"

//任务队列的结点
typedef struct node
{
    int clientFd;
    struct node* pNext;
}node_t,*pNode_t;

//队列
typedef struct
{
    pNode_t pHead,pTail;
    int size;
    pthread_mutex_t mLock;//队列要互斥访问
    pthread_cond_t cond;
}que_t,*pQue_t;

int queInit(pQue_t);
int queInsert(pQue_t,pNode_t);
int queGet(pQue_t,pNode_t*);

#endif
