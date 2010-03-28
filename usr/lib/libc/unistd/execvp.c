/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest objęty licencją, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id: execve.c 625 2009-07-09 10:17:32Z takeshi $
 */
#include <sys/syscall.h>
#include <unistd.h>
#include <paths.h>
#include <string.h>
#include <errno.h>

#include "libc_syscall.h"

int
execvp(const char *file, char *const argv[])
{
    int res = 0;
    if(strchr(file, '/')) //full path
        return execve(file, argv, environ);
    size_t flen = strlen(file);
    char *paths = getenv("PATH");
    if(!paths)
        paths = _PATH_DEFPATH;
    paths = strdup(paths);
    char *path = NULL;
    while((path = strsep(&paths, ":")) != NULL) {
        char *npath = malloc(flen + strlen(path) + 2);
        sprintf(npath, "%s/%s", path, file);
        res = execve(npath, argv, environ);
        if(errno == EACCES || errno == ENOENT)
            continue;
        if(errno == ENOEXEC) {
            int argc = 0;
            while(argv[argc++])
                ;
            char **altargv = malloc(sizeof(char*) * (argc+1));
            for(int i=0; i<argc; i++)
                altargv[i+1] = argv[i];
            altargv[1] = npath;
            altargv[0] = "sh";
            res = execve("/bin/sh", altargv, environ);
            free(paths);
            free(altargv);
            return res;
        }
    }
    free(paths);
    return -1;
}

