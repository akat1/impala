/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest obj�ty licencj�, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#include <unistd.h>
#include <sys/ioctl.h>

pid_t
tcgetpgrp(int fd)
{
    return ioctl(fd, TIOCGPGRP);
}
