#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/utsname.h>

int
uname(struct utsname *un)
{
	if(syscall(SYS_uname, un)) return -1;
    strcat(un->nodename, "czarny_specjal");
    return 0;
}
