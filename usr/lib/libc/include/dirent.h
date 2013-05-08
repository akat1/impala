/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
 * All rights reserved.
 *
 * Niniejszy plik jest objęty licencją, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#ifndef __DIRENT_H
#define __DIRENT_H

#include <sys/types.h>

struct DIR {
    int flags;
    int fd;
};

typedef struct DIR DIR;

// jakoś inaczej to trzeba zrobić
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
