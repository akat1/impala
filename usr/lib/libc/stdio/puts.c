#include <stdio.h>
#include <fcntl.h>


int
puts(const char *str)
{
    return fputs(str, stdout);
}