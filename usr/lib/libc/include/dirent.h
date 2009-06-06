#ifndef __DIRENT_H
#define __DIRENT_H

#include <sys/types.h>

typedef int DIR;

// jako¶ inaczej to trzeba zrobiæ
#define MAX_NAME 128

struct dirent {
    int     d_ino;
    char    d_name[MAX_NAME];
};

typedef struct dirent dirent_t;


int       closedir(DIR *);
DIR      *opendir(const char *);
dirent_t *readdir(DIR *);
//int     readdir_r(DIR *, struct dirent *,
//                        struct dirent **);
//void    rewinddir(DIR *);
//void    seekdir(DIR *, long);
//long    telldir(DIR *);


#endif