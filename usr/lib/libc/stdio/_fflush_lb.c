#include <stdio.h>
#include <stdio_private.h>

void
__fflush_line_buffered(void)
{
    FILE *f = NULL;
    while((f = list_next(&__open_files, f))) {
        if(ISSET(f->status, _FST_LINEBUF) && ISSET(f->status, _FST_OPEN))
            fflush(f);
    }
}
