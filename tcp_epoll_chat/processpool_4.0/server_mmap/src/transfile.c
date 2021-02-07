#include "process_pool.h"

#define FILENAME "file"

int transFile(int clientFd)
{
    int fd=open(FILENAME,O_RDWR);
    ERROR_CHECK(fd,-1,"open");
    
    //发送文件名
    train_t train;
    //头部长度
    int headLen=sizeof(train.dataLen);
    memset(&train,0,sizeof(train));
    strcpy(train.buf,FILENAME);
    //数据长度
    train.dataLen=strlen(FILENAME);
    int ret=send(clientFd,&train,headLen+train.dataLen,0);
    ERROR_CHECK(ret,-1,"sendname");

    //发送文件大小
    struct stat filestat;
    memset(&filestat,0,sizeof(filestat));
    ret=fstat(fd,&filestat);
    ERROR_CHECK(ret,-1,"fstat");
    memcpy(train.buf,&filestat.st_size,sizeof(filestat.st_size));
    train.dataLen=sizeof(filestat.st_size);
    ret=send(clientFd,&train,headLen+train.dataLen,0);
    ERROR_CHECK(ret,-1,"sendsize");
    

    //发送文件内容,mmap
    char* pMap=(char*)mmap(NULL,filestat.st_size,PROT_READ|PROT_WRITE,
                           MAP_SHARED,fd,0);
    ERROR_CHECK(pMap,(char*)-1,"mmap");

    ret=send(clientFd,pMap,filestat.st_size,0);
    ERROR_CHECK(ret,-1,"sendfile");

    munmap(pMap,filestat.st_size);

    close(fd);
    return 0;
}

