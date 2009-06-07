#include <stdlib.h>
#include <unistd.h>

void*
malloc(size_t size)
{
    ///@todo impl
    int ns = size+sizeof(size_t);
    void* addr = sbrk(ns)-size;
    ((int*)addr)[-1] = size;
    return addr;
}