#include <unistd.h>
#include <sys/syscall.h>

void*
sbrk(ptrdiff_t incr)
{
    return (void*)syscall(SYS_sbrk, incr);
}