/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest objêty licencj±, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

DIR*
opendir(const char *dname)
{
    struct stat s;
    int err = stat(dname, &s);
    if(err)
        return NULL;
    if(!S_ISDIR(s.st_mode)) {
        errno = ENOTDIR;
        return NULL;
    }
    DIR *res = malloc(sizeof(DIR));
    if(!res) {
        errno = ENOMEM;
        return NULL;
    }
    res->fd = open(dname, O_RDONLY);
    if(res->fd < 0) {
        free(res);
        return NULL;
    }
    return res;
}
