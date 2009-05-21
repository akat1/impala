#include <sys/types.h>
#include <string.h>

size_t
strlen(const char *str)
{
    size_t i;
    for (i = 0; *str; i++) str++;
    return i;
}
