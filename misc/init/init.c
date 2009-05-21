#include <sys/syscall.h>

void
atexit()
{
    
}

void
syscall(int sc, ...) {
    asm (
        "movl %%ebx, %%eax;"
        " int $0x80"
        : : "b"(sc)
    );
}

int
strlen(const char *str)
{
    int i;
    for (i = 0; *str; str++, i++);
    return i;
}

void
output(const char *str)
{
    syscall(SYS_write, 1, str, strlen(str));
}

char hello[] = "Hello, World\n";

int
main(int argc, char **argv)
{
    output("init process\n");
    output(hello);
    return 0;
}

void
_start()
{
    main(0,0);
    for(;;);
}


