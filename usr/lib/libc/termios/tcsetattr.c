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
#include <termios.h>
#include <sys/ioctl.h>


int
tcsetattr(int fd, int optact, const struct termios *termios_p)
{
    if(optact == TCSANOW)
        return ioctl(fd, TCSETS, termios_p);
    else
        return ioctl(fd, TCSETS, termios_p);///@todo zrobiæ normalnie
}