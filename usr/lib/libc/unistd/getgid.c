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
#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

gid_t
getgid(void)
{
	return syscall(SYS_getgid);
}
