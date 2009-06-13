#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

int
pipe(int filedes[2])
{
    return syscall(SYS_pipe, filedes);
}
