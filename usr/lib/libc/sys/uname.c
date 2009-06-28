#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <string.h>

int
uname(struct utsname *un)
{
	syscall(SYS_uname, un);
    strcat(un->nodename, "czarny_specjal");
    return 0;
}
