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
#include <unistd.h>
#include <dirent.h>
#include <sys/syscall.h>

dirent_t _dir;

struct dirent *
readdir(DIR *dirp)
{
    int res = syscall(SYS_getdents, dirp->fd, &_dir, sizeof(struct dirent));
    if(res == 0)
        return NULL;
    return &_dir;
}
