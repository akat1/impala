/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mount.h>

int mount(const char *type, const char *dir, int flags, void *data)
{
	return syscall(SYS_mount, type, dir, flags, data);
}
