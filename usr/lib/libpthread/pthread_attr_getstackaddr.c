#include <sys/types.h>
#include <pthread.h>

int
pthread_attr_getstackaddr(const pthread_attr_t *a, void **addr)
{
    *addr = a->stackaddr;
    return 0;
}
