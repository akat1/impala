#include <sys/utils.h>
#include <sys/kprintf.h>

void
panic(const char *msg)
{
        __asm__ ("cli");
        kprintf("\n\nKernel Panic !!!\nError: %s\n\n", msg);
        while(1);
}
