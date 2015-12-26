#include <unistd.h>
#include "rio.h"
#include <semaphore.h>

volatile int buf = 0;
int max = 10;
sem_t mutex;
sem_t empty;
sem_t full;

// 不用semaphore来表示empty和full其实也可以。只是会导致在while loop空转
// 效率不高。
void* producer_nonblock(void* args){
    int id = *((int*)args);
    free((int*)args);
    pthread_detach(pthread_self());
    debug("producer start");
    while(1){
        if(sem_wait(&mutex) < 0){
            printError("set wait error");
        }
        if(buf < max){
            buf += 1;
            debug("produce one product, buf now is: %d", buf);
        }
        if(sem_post(&mutex) < 0)
            printError("set post error");
        usleep(rand()%10000 + 10000);
    }
    return NULL;
}

void* consumer_nonblock(void* args){
    int id = *((int*)args);
    free((int*)args);
    pthread_detach(pthread_self());
    debug("consumer start");
    while(1){
        sem_wait(&mutex);
        if(buf >0){
            buf -= 1;
            debug("comsume one product, buf now is: %d", buf);
        }
        sem_post(&mutex);
        usleep(rand()%10000 + 10000);
    }
    return NULL;
}

void* producer(void* args){
    int id = *((int*)args);
    int fullvalue;
    debug("producer start");
   // if(pthread_detach(pthread_self()) < 0)
   //     printError("detach error");
    debug("detach finished");
    free(args);
    while(1){
        //sem_wait是原子操作，所以不用担心empty的同步问题
        //要注意是先require empty，再require锁，否则获得了锁却被empty block住的话，就会block住所有等待锁的进程
        sem_wait(&empty);
        //实践中empty和full对了是一些实际存储的共享数据结构，所以这里需要加锁
        sem_wait(&mutex);
        sem_getvalue(&full, &fullvalue);
        debug("%d produce one product, buf now is: %d", id,  fullvalue + 1);
        sem_post(&mutex);
        sem_post(&full);
        usleep(rand()%10000 + 10000);
    }
    return NULL;
}

void* consumer(void* args){
    int id = *((int*)args);
    int fullvalue;
    debug("consumer start");
   // if(pthread_detach(pthread_self()) < 0)
   //     printError("detach error c");
    debug("detach finished");
    free(args);
    while(1){
        sem_wait(&full);
        sem_wait(&mutex);
        sem_getvalue(&full, &fullvalue);
        debug("%d consume one product, buf now is %d", id,fullvalue);
        sem_post(&mutex);
        sem_post(&empty);
        usleep(rand()%10000 + 10000);
    }
    return NULL;
}

int bufSize = 10;
int main(){
    debug("main start");
    if(sem_init(&mutex, 0, 1) < 0 )
        printError("create mutex error");
    if(sem_init(&full, 0, 0) < 0 )
        printError("create mutex error");
    if(sem_init(&empty, 0, bufSize) < 0 )
        printError("create mutex error");
    int N = 10;
    pthread_t tidp[10];
    pthread_t tidc[10];
    int i = 0;
    for(i = 0; i < N; i++){
        int* id = malloc(sizeof(int));
        *id = i;
        if(pthread_create(tidp+i, NULL, producer, id) < 0){
            printError("create thread producer error");
        }
        pthread_detach(*(tidp+i));
    }
    i = 0;
    for(i = 0; i < N; i++){
        int* id = malloc(sizeof(int));
        *id = i;
        if(pthread_create(tidc+i, NULL, consumer,id) < 0){
            printError("Create thread comsumer error");
        }
        pthread_detach(*(tidc+i));
    }
 // 如果没有detach的话，就要用join来回收资源以及阻塞主线程 否则主线程一返回，整个进程就结束了
    //pthread_join(tidp[0]);
    pthread_exit(NULL); // 主线程调用这个函数，回等待所有线程结束才返回
}
 
