#include <sys/types.h>
#include <sys/device.h>

d_init_t    pata_init;
d_init_t    md_init;
d_init_t    fdc_init;
d_init_t    null_init;
d_init_t    zero_init;

d_init_t    *devtab[] = {
    md_init,
    fdc_init,
    pata_init,
    null_init,
    zero_init,
    NULL
};
