#include <stdarg.h>
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

void debug(char* format,...){
    va_list args;
    va_start(args,format);
    vprintf(format,args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}


void printError(char* s){
    fprintf(stderr, "%s error, errorcode: %d\n", s, errno);
    printBacktrace();
    fflush(stderr);
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
        debug("rio read internal buf start");
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        debug("rio read %d bytes to internal buf", rp->rio_cnt);
        if(rp->rio_cnt < 0)
            return -1; // error
        else if(rp->rio_cnt == 0)
            return 0; //EOF
        rp->rio_bufptr = rp->rio_buf;
        debug("rio read one loop end");
    }

    cnt = n;
    if(n > rp->rio_cnt)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_cnt -= cnt;
    rp->rio_bufptr += cnt;
    //debug("rio read %d bytes in one call", cnt);
    return cnt;
}

ssize_t rio_readlineb(rio_t* rp, void* usrbuf, size_t maxlen){
    int n, rc;
    char c, *bufp = usrbuf;

    for(n = 1; n < maxlen; n++){
        if((rc = rio_read(rp, &c, 1)) == 1){
            *bufp++ = c;
//            debug("%c",c);
            if(c == '\n')
                break;
        }else if(rc == 0)
            return 0;
        else
            return -1; // error
    }
    *bufp = 0;
    debug("rio read one line");
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

int open_listenfd(int port){
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;
    
    if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
        return -1;
    
    debug("create socket, fd = %d ", listenfd);

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                (const void*)&optval,sizeof(int)) < 0)
        return -1;

    debug("set socket opt ok");
    debug("port %d", port);
    
    bzero((char*)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //inet_pton(AF_INET, "192.168.31.111", &serveraddr.sin_addr);
    serveraddr.sin_port = htons((unsigned short)port);
    
    if(bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    debug("bind socket ok");

    if(listen(listenfd,100) < 0)
        return -1;

    debug("listen ok, fd = %d", listenfd);

    return listenfd;

}

void doit(int fd);
void read_requestthdrs(rio_t* rp);
int parse_uri(char* url, char* filename, char* cgiargs);
void serve_static(int fd, char* filename, int filesize);
void serve_dynamic(int fd, char* filename, char* cgiargs);
void get_filetype(char* filename, char* filetype);
void clienterror(int fd, char* cause, char* errnum,
        char* shortmasg, char* longmsg);


int main(int argc, char** argv )
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    if(argc != 2){
        fprintf(stderr, "usage: %s <port>\n",argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);
    
    if((listenfd = open_listenfd(port)) < 0){
        debug("open listenfd error errno = %d ", errno);
    }

    debug("listenfd = %d ", listenfd);

    while(1){
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd,(SA*)&clientaddr, &clientlen);
        if(connfd == -1)
            printError("accept error");
        debug("get a request from %d ", clientaddr.sin_port);
        doit(connfd);
        close(connfd);
        debug("a connection closed");
    }


    return 0;
}

void doit(int fd){
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];

    rio_t rio;
    rio_readinitb(&rio, fd);

    Rio_readlineb(&rio,buf, MAXLINE); // read the http request
    debug("get a request %s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if(strcasecmp(method, "GET")){
        clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not Implemented this method");
        return;
    }

    read_requestthdrs(&rio);
    
    is_static = parse_uri(uri, filename, cgiargs);
    if(stat(filename, &sbuf) < 0){
        clienterror(fd, filename, "404", "NOT found",
                "Tiny coudln't find this file");
        return;
    }

    if(is_static){
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
            clienterror(fd, filename, "403", "Forbidden", 
                    "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }else{
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
            clienterror(fd, filename, "403", "Forbidden", 
                    "Tiny could't run the CGI program");
            return;
        }
        serve_dynamic(fd,filename,cgiargs);
    }   
}


void clienterror(int fd, char* cause, char* errnum,
        char* shortmasg, char* longmsg){
    char buf[MAXLINE], body[MAXLINE];

    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body, "%s%s: %s\r\n",body, errnum, shortmasg);
    sprintf(body, "%s<p>%s:%s\r\n",body,longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n",body);

    sprintf(buf, "HTTP/1.0 %s %s \r\n", errnum, shortmasg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void read_requestthdrs(rio_t *rp){
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")){
        Rio_readlineb(rp,buf,MAXLINE);
        debug("a header line: %s",buf);
    }
    return;
}


int parse_uri(char* uri, char* filename, char* cgiargs){
    char* ptr;

    if(!strstr(uri, "cgi-bin")){
        strcpy(cgiargs, "");
        strcpy(filename,"./static");
        strcat(filename,uri);
        if(uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }else{
        ptr =  index(uri, '?');
        if(ptr){
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
            debug("get arguments uri now is %s",uri);
        }
        else{
            strcpy(cgiargs,"");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

void serve_static(int fd, char* filename, int filesize){
    int srcfd;
    char* srcp, filetype[MAXLINE], buf[MAXLINE];
    debug("call serve_static with filename %s", filename);
    //send response headers to client
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 ok\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  //  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    // send response body to client
    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd,0);
    close(srcfd);
    sleep(3);
    Rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

void get_filetype(char* filename, char* filetype){
    if(strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if(strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if(strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char* filename, char* cgiargs){
    char buf[MAXLINE], *emptylist[]= {NULL};

  //  sprintf(buf, "HTTP/1.0 200 ok\r\n");
  //  Rio_writen(fd, buf, strlen(buf));
  //  sprintf(buf, "Server:Tiny Web Server\r\n");
  //  Rio_writen(fd,buf,strlen(buf));
  //  sprintf("Content-length: %u\r\n", strlen(content)+1);
  //  printf("Content-type: %s\r\n\r\n", "text/html");

    sprintf(buf, "HTTP/1.0 200 ok\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
   // sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, "text/html");
    //sprintf(buf, "%s hello this is a dynamic service\n");
    Rio_writen(fd, buf, strlen(buf));


    if(fork() == 0){
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd,STDOUT_FILENO);
        if(execve(filename, emptylist, environ) < 0)
            printError("execve error");
    }
    wait(NULL);
}

