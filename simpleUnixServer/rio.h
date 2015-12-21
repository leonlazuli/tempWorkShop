#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>


#define MAXLINE 1000
typedef struct sockaddr SA;

extern char** environ;

void printBacktrace(){
    
   int j, nptrs;
   #define SIZE 10000
   void *buffer[100];
   char **strings;

   printf("---------------------------\n");

   nptrs = backtrace(buffer, SIZE);
   printf("backtrace() returned %d addresses\n", nptrs);

   /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
      would produce similar output to the following: */

   strings = backtrace_symbols(buffer, nptrs);
   if (strings == NULL) {
       perror("backtrace_symbols");
       exit(EXIT_FAILURE);
   }

   for (j = 0; j < nptrs; j++)
       printf("%s\n", strings[j]);

   printf("---------------------------\n");
   free(strings);
}

void debug(char* s){
    printf("%s\n", s);
}

void printError(char* s){
    fprintf(stderr, "%s error, errorcode: %d\n", s, errno);
    printBacktrace();
    exit(1);
}

ssize_t rio_readn(int fd, void* userbuf, size_t n){
    size_t nleft = n;
    size_t nread;
    char* *bufp = userbuf;
    while(nleft >0){
        if((nread = read(fd,bufp,nleft)) < 0){
            return -1; // error
        }else if(nread == 0)
           break; //EOF
        nleft -= nread;
        bufp += nread;
    }
    return (n-nleft);
}

ssize_t rio_writen(int fd, void* usrbuf, size_t n){
    size_t nleft = n;
    size_t nwrriten ;
    char* buf = usrbuf;
    while(nleft > 0){
        if((nwrriten = write(fd,buf,nleft)) < 0)
            return -1; // error
        nleft -= nwrriten;
        buf += nwrriten;
    }
    return n;
}

#define RIO_BUFSIZE 8192
typedef struct{
    int rio_fd;
    int rio_cnt;
    char* rio_bufptr;
    char rio_buf[RIO_BUFSIZE];

} rio_t;

void rio_readinitb(rio_t* rp, int fd){
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_read(rio_t* rp, char* usrbuf, size_t n){
    int cnt;
    while(rp->rio_cnt <= 0){
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0)
            return -1; // error
        else if(rp->rio_cnt == 0)
            return 0; //EOF
        rp->rio_bufptr = rp->rio_buf;
    }

    cnt = n;
    if(n > rp->rio_cnt)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_buf, cnt);
    rp->rio_cnt -= cnt;
    rp->rio_bufptr += cnt;
    return cnt;
}

ssize_t rio_readlineb(rio_t* rp, void* usrbuf, size_t maxlen){
    int n, rc;
    char c, *bufp = usrbuf;

    for(n = 1; n < maxlen; n++){
        if((rc = rio_read(rp, &c, 1)) == 1){
            *bufp++ = c;
            if(c == '\n')
                break;
        }else if(rc == 0)
            return 0;
        else
            return -1; // error
    }
    *bufp = 0;
    return n;
}

// wapper functions
ssize_t Rio_read(rio_t* rp, char* usrbuf, size_t n){
    ssize_t ret = rio_read(rp, usrbuf, n);
    if(ret < 0){
        printError("rio_read error");
    }
    return ret;
}

ssize_t Rio_writen(int fd, void* usrbuf, size_t n){
    ssize_t ret = rio_writen(fd, usrbuf, n);
    if(ret < 0){
        printError("rio_writen error");
    }
    return ret;
}

ssize_t Rio_readn(int fd, void* usrbuf, size_t n){
    ssize_t ret =rio_readn(fd, usrbuf, n);
    if(ret < 0){
        printError("rio_readn error");
    }
    return ret;
}


ssize_t Rio_readlineb(rio_t* rp, void* usrbuf, size_t maxlen){
    ssize_t ret =rio_readlineb(rp, usrbuf, maxlen);
    if(ret < 0){
        printError("rio_readlineb error");
    }
    return ret;
}
// wrapper funtions end

