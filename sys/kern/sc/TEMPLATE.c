/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vm.h>

typedef struct XXX_args XXX_args_t;
struct XXX_args {
    int fd;
};

int sc_XXX(thread_t *, syscall_result_t *, XXX_args_t *);

int
sc_XXX(thread_t *t, syscall_result_t *r, XXX_args_t *args)
{
    return 0;
}
