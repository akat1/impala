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
#ifndef __TIME_H
#define __TIME_H

#include <sys/types.h>
#include <sys/time.h>


struct tm {
    int tm_sec;     /* 0 - 60 */
    int tm_min;     /* 0 - 59 */
    int tm_hour;    /* 0 - 23 */
    int tm_mday;    /* 1 - 31 */
    int tm_mon;     /* 0 - 11 */
    int tm_year;    /* 1900 */
    int tm_wday;    /* niedziela to 0  */
    int tm_yday;    /* 0 - 365 */
    int tm_isdst; 
    char *tm_zone; 
    long tm_gmtoff; 
};

int nanosleep(const timespec_t *req, timespec_t *el);
time_t time(time_t *);

struct tm *localtime(const time_t *);
struct tm *localtime_r(const time_t *, struct tm *);

const char *ctime(const time_t *);

#endif
