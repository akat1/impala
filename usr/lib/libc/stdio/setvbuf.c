#include <stdio.h>

int
setvbuf(FILE *f, char *b, int mode, size_t size)
{
    if(f->buf != NULL)
        return -1;  //ju¿ nie mo¿na zmieniaæ -> kto¶ wykona³ I/O na f
    
    
}