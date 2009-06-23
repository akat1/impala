#include <stdlib.h>
#include <unistd.h>

void*
malloc(size_t size)
{
    ///@todo impl
    int ns = size+sizeof(int*);
    void* addr = sbrk(ns)-size;
    ((int*)addr)[-1] = size;
    return addr;
}