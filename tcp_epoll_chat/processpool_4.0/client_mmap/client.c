#include <func.h>

int recvCycle(int fd,void *buf,size_t len);

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);
    
    int serverFd = socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(serverFd,-1,"socket");

    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serAddr.sin_port = htons(atoi(argv[2]));
 
    //connect连接服务器
    int ret = connect(serverFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"connect");


    char buf[1000]={0};
    //1.接收文件名
    int dataLen=0;
    //先接收火车头
    recv(serverFd,&dataLen,sizeof(int),MSG_WAITALL);
    //再接收车厢
    recv(serverFd,buf,dataLen,MSG_WAITALL);

    int fd=open(buf,O_RDWR|O_CREAT,0666);
    ERROR_CHECK(fd,-1,"open");
    printf("recv filename=%s\n",buf);

    //2.接收文件大小
    off_t fileSize=0;
    recv(serverFd,&dataLen,sizeof(int),MSG_WAITALL);
    recv(serverFd,&fileSize,dataLen,MSG_WAITALL);
    printf("file size=%ld\n",fileSize);

    //设置文件大小
    ftruncate(fd,fileSize);

    struct timeval start,mid,end;
    memset(&start,0,sizeof(start));
    memset(&end,0,sizeof(end));
    memset(&mid,0,sizeof(mid));
    gettimeofday(&start,NULL);
    memcpy(&mid,&start,sizeof(start));

    off_t downSize=0;

    //3.接收文件内容，写入到文件中
    char* pMap = (char*)mmap(NULL,fileSize,PROT_READ|PROT_WRITE,
                             MAP_SHARED,fd,0);
    ERROR_CHECK(pMap,(char*)-1,"mmap");

    recvCycle(serverFd,pMap,fileSize);

    gettimeofday(&end,NULL);
    printf("cost time=%ld\n",
           (end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
    
    munmap(pMap,fileSize);

    close(fd);
    close(serverFd);
    return 0;
}

