#ifndef __STDIO_PRIVATE_H__
#define __STDIO_PRIVATE_H__

#include <sys/list.h>

void __check_buf(FILE *f);
int  __put_char(FILE *f, char c);
int  __put_str(FILE *f, const char *str);
int  __put_data(FILE *f, const char *str, size_t size);
void __fflush_line_buffered(void);


extern list_t __open_files;

#endif
