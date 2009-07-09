#ifndef __STDIO_PRIVATE_H__
#define __STDIO_PRIVATE_H__

//#ifdef 1 || __STDIO_PRIV

#include <sys/list.h>

struct FILE {
    int      fd;
    void    *cookie;
    int    (*readfn)(void *, char *, int);
    int    (*writefn)(void*, const char *, int);
    fpos_t (*seekfn)(void *, fpos_t, int);
    int    (*closefn)(void *);
    int      status;
    int      err;
    char    *buf;
    size_t   buf_size;
    size_t   inbuf;
    list_node_t L_open_files;
};

void __check_buf(FILE *f);
int  __put_char(FILE *f, char c);
int  __put_str(FILE *f, const char *str);
int  __put_nstr(FILE *f, const char *str, int maxlen);
int  __put_data(FILE *f, const char *str, size_t size);
int  __get_data(FILE *f, char *str, size_t size);
void __fflush_line_buffered(void);


extern list_t __open_files;
//#endif

#endif
