#include <pthread.h>


int
pthread_attr_setstacksize(pthread_attr_t *a, size_t s)
{
    a->stacksize = s;
    return 0;
}
