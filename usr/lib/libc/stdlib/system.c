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
 * $Id$
 */ 
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int
system(const char *command)
{
    switch(fork()) {
        case -1:
            return -1;
        case 0: {
            execl("/bin/sh", "sh", "-c", command, NULL);
            break;
        }
        default: {
            int status = 0;
            waitpid(-1, &status, 0);
            return WEXITSTATUS(status);
        }
    }
    return -1;
}
