#include "../include/workque.h"

//初始化队列
int queInit(pQue_t pQue)
{
    memset(pQue,0,sizeof(que_t));

    pQue->pHead=NULL;
    pQue->pTail=NULL;
    pQue->size=0;
    pthread_mutex_init(&pQue->mLock,NULL);
    pthread_cond_init(&pQue->cond,NULL);

    return 0;
}

//队尾入队
int queInsert(pQue_t pQue,pNode_t pNew)
{
    if(0==pQue->size)//空队列
    {
        pQue->pHead=pNew;
        pQue->pTail=pNew;
    }
    else
    {
        pQue->pTail->pNext=pNew;
        pQue->pTail=pNew;
    }
    pQue->size++;
    return 0;
}

//队头出队
int queGet(pQue_t pQue,pNode_t* pGet)
{
    if(0==pQue->size)//空队列
    {
        return -1;
    }

    *pGet=pQue->pHead;
    pQue->pHead=pQue->pHead->pNext;

    if(NULL==pQue->pHead)//出队的是最后一个元素
    {
        pQue->pTail=NULL;
    }

    pQue->size--;

    return 0;
}
