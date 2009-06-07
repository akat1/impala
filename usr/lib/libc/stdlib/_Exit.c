#include <sys/types.h>
#include <sys/syscall.h>
#include <stdlib.h>

void
_Exit(int status)
{
    syscall(SYS_exit, status);
}

//mam nadziejê, ¿e nie zostanê pobity za dwie funkcje w 1 pliku... ;p
void
_exit(int status)
{
    syscall(SYS_exit, status);
}
