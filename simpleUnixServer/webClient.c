#include "rio.h"

int open_clientfd(char* hostname, int port){
    int clientfd;
    struct sockaddr_in serveraddr;

    if((clientfd = socket(AF_INET,SOCK_STREAM, 0)) < 0)
        return -1;

    bzero((char*)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, hostname, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(port);

    if(connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

int main(int argc, char** argv){
    int clientfd, port;
    char buf[MAXLINE];
    char* host;
    rio_t rio;

    if(argc != 4){
        fprintf(stderr, "usage: %s <host> <port> <request>\n",argv[0]);
        exit(0);
    }
    host = argv[1];
    port = atoi(argv[2]);

    clientfd = open_clientfd(host,port);
    if(clientfd  <= 0)
        printError("connect to server failed");
    debug("connect success");
    
    rio_readinitb(&rio, clientfd);
    sleep(3);
    sprintf(buf,"GET %s HTTP/1.0\r\n\r\n", argv[3]);
    debug("%s", buf);
    Rio_writen(clientfd, buf, strlen(buf));

    while(Rio_readlineb(&rio, buf,MAXLINE)){
        fputs(buf,stdout);
        fflush(stdout);
    }
    debug("client exit");
    
    
}
