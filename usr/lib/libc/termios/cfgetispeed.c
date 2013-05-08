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
 * $Id: cfgetospeed.c 625 2009-07-09 10:17:32Z takeshi $
 */
#include <sys/types.h>
#include <termios.h>


speed_t
cfgetispeed(const struct termios *termios_p)
{
    return termios_p->c_ispeed;
}
