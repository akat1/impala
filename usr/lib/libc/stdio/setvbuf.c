#include <stdio.h>

int
setvbuf(FILE *f, char *b, int mode, size_t size)
{
    if(f->buf != NULL)
        return -1;  //ju� nie mo�na zmienia� -> kto� wykona� I/O na f
    
    
}