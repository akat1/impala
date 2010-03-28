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
#ifndef __GRP_H__
#define __GRP_H__

struct group {
    char    *gr_name;
    char    *gr_passwd;
    gid_t    gr_gid;
    char   **gr_mem;
};



struct group *getgrnam(const char *name);
struct group *getgrgid(gid_t gid);




#endif
