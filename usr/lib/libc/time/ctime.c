#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <time.h>



const char *
ctime(const time_t *t)
{
    static char ctext[26];
    snprintf(ctext, sizeof(ctext), "[fakectime %u]", *t);
    return ctext;
}
