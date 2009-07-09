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
    return 0;
}

int
main()
{
    pthread_t thread;
    pthread_create(&thread, NULL, tmain, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);

    pthread_mutex_destroy(&mutex);
    return 0;
}
