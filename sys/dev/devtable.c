#include <sys/types.h>
#include <sys/device.h>

d_init_t    md_init;

d_init_t    *devtable[] = {
    md_init,
    NULL
};
