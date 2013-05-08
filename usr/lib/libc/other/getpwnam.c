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
#include <pwd.h>

struct passwd onlyuser = {
    .pw_name = "toor",
    .pw_uid = 0,
    .pw_gid = 0,
    .pw_dir = "/",
    .pw_shell = "/bin/sh"
};

struct passwd *
getpwnam(const char *name)
{
    return &onlyuser;   ///@todo impl. getpwnam
}
