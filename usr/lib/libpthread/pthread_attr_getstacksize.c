#include <sys/types.h>
#include <pthread.h>


int
pthread_attr_getstacksize(const pthread_attr_t *a, size_t *s)
{
    *s = a->stacksize;
    return 0;
}

