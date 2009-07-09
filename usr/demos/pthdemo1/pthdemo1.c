#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static void *tmain(void *_);
int main(void);

void *
tmain(void *_)
{
    for (int i = 0; i < 5; i++) {
        printf("\033[2K\rI am alive! %i/4", i);
        fflush(stdout);
        sleep(1);
    }
    printf("\033[2K\r * end\n");
    return "working well";
}

int
main()
{
    void *retval;
    pthread_t thread;
    printf(" * creating thread\n");
    pthread_create(&thread, NULL, tmain, NULL);
    pthread_join(thread, &retval);
    printf(" * thread retval is \"%s\"\n", retval);
    return 0;
}
