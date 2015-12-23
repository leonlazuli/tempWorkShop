#include "../rio.h"

extern char** environ;

int main(){
    char* p = NULL;
    char* query = getenv("QUERY_STRING");
    char arg1[MAXLINE], arg2[MAXLINE];
    char content[MAXLINE];
    int n1 = 0;
    int n2 = 0;

    fprintf(stderr,"------------ adder is executed, query is %s\n", query); // just for testing, should not be error

    if(query == NULL)
        fprintf(stderr, "----------query string is null");
    
    p = strchr(query,'&');
    *p = '\0';
    strcpy(arg1,query);
    strcpy(arg2,p+1);
    n1 = atoi(arg1);
    n2 = atoi(arg2);
    
//## 这里写啥无所谓，但是一定要有个\r\n\r\n 来作为http header结束的标记，否则浏览器不会显示主体部分
    printf("Content-type: \r\n\r\n"); 
    sprintf(content,"-----------welcome to use Leon's server!\n %d + %d = %d\n", n1,n2,n1+n2); 
    printf("%s",content);
    fprintf(stderr,"%s", content); 
    fflush(stderr);

    return 0;
}
