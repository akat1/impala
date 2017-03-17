/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#ifndef __SYS_MOUNT_H
#define __SYS_MOUNT_H
#ifndef __KERNEL
int mount(const char *type, const char *dir, int flags, void *data);
#endif /* ! __KERNEL */
#endif /* ! __SYS_MOUNT_H */
