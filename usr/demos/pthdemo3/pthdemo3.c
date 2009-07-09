#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static void *treader(void *_);
static void *twriter(void *_);

int main(void);

static pthread_mutex_t mutex;
static pthread_cond_t cond;
static int counter = 0;
static int quit = 0;

void *
treader(void *_)
{
    sleep(6);
    while (!quit) {
        pthread_mutex_lock(&mutex);
        while (counter == 0 && !quit) {
            printf("[%2u] treader%u: no messages in queue, waiting\n", counter, _);
            pthread_cond_wait(&cond, &mutex);
        }
        if (!quit) {
            printf("[%2u] treader%u: getting message from queue\n", counter--, _);
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void *
twriter(void *_)
{
    for (int i = 0; i < 8; i++) {
        pthread_mutex_lock(&mutex);
        printf("\033[32m[%2u] twriter%u: sending %u\033[0m\n", counter, _, i);
        counter++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
    pthread_mutex_lock(&mutex);
    printf("[  ] twriter%u: end\n", _);
    quit = 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}


int
main()
{
    pthread_t thread1, thread2, thread3;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_mutex_lock(&mutex);
    pthread_create(&thread1, NULL, treader, (void*)1);
    pthread_create(&thread2, NULL, treader, (void*)2);
    pthread_create(&thread3, NULL, twriter, (void*)1);
    pthread_mutex_unlock(&mutex);
    pthread_join(thread1, 0);
    pthread_join(thread2, 0);
    pthread_join(thread3, 0);

    return 0;
}
