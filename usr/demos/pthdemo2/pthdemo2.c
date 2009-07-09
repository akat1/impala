#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static void *tmain(void *_);
int main(void);

static pthread_mutex_t mutex;

void *
tmain(void *_)
{
    printf("thread%u: trying to enter critical section\n", _);
    pthread_mutex_lock(&mutex);
    printf("thread%u: in critical ssection\n", _);  
    sleep(2);
    printf("thread%u: leaving\n", _);
    pthread_mutex_unlock(&mutex);
    return 0;
}

int
main()
{
    pthread_t thread1, thread2;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    printf("main: creating threads and waiting 5 seconds\n");
    pthread_create(&thread1, NULL, tmain, (void*)1);
    pthread_create(&thread2, NULL, tmain, (void*)2);
    sleep(5);
    printf("main: unlocking \n");
    pthread_mutex_unlock(&mutex);
    printf("main: waiing for threads\n");
    pthread_join(thread1, 0);
    pthread_join(thread2, 0);
    pthread_mutex_destroy(&mutex);
    return 0;
}
