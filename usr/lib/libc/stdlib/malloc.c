#include <stdlib.h>
#include <unistd.h>

void*
malloc(size_t size)
{
    ///@todo impl
    return sbrk(size)-size;
}