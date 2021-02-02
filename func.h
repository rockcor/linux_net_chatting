#include <stdio.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define PERM S_IRUSR|S_IWUSR
#define ARGS_CHECK(argc,val) {if(argc!=val)  {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcname) {if(retval==ret){perror(funcname);return -1;}}
#define THREAD_ERRORCHECK(ret,funName)    do{ if( 0 != ret) { printf("%s : %s\n", funName, strerror(ret) ); } }while(0)
