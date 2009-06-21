#include <sys/types.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio_private.h>

void
exit(int status)
{
    FILE *f = NULL;
    while ((f = list_head(&__open_files)))
        fclose(f);
    syscall(SYS_exit, status);
}
