#include <sys/types.h>
#include <sys/syscall.h>
#include <stdlib.h>

void
exit(int status)
{
    syscall(SYS_exit, status&0377); //to &0377 tutaj czy w sc_exit?
}
