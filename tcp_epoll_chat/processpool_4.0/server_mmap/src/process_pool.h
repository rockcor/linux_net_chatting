#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>


#define ARGS_CHECK(argc,val) {if(argc!=val)  {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcname) {if(retval==ret){perror(funcname);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcname) do{if(0!=ret){printf("%s : %s\n",funcname,strerror(ret));}}while(0)

//子进程信息
typedef struct
{
    pid_t pid;
    int pipefd;
    short busy;
}ProcessData_t,*pProcessData_t;

//小火车协议
typedef struct
{
    int dataLen;
    char buf[1000];
}train_t;

int makeChild(int,pProcessData_t);
int childFunc(int);
int sendFd(int,int,char);
int recvFd(int,int*,char*);
int tcpInit(int*,char*,char*);
int epollAdd(int,int);
int transFile(int);
