#include <sys/types.h>
#include <pthread.h>

int
pthread_attr_setstackaddr(pthread_attr_t *a, void *addr)
{
    a->stackaddr = addr;
    return 0;
}

