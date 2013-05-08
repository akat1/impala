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
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>

char *optarg;
int optind = 1, opterr = 0, optopt, optreset;

#define UNKNOWN_OPT ((int)'?')
#define MISSING_ARG ((int)':')

int
getopt(int argc, char * const argv[], const char *optstring)
{
    static char *p = "";
    char *opt;

    /* jesteśmy na końcu */
    if ( *p == '\0' || optreset ) {
        if ( optind >= argc ) {
            p = "";
            return -1;
        }

        p = argv[optind];

        /* oczekujemy opcji */
        if ( p[0] != '-' ) {
            p = "";
            return -1;
        }

        /* jeżeli dostaliśmy --, to koniec */
        if ( p[0] == '-' && p[1] == '-' ) {
            p = "";
            return -1;
        }

        /* dostaliśmy jakąś opcje */
        optind++;
        p++;
    }

    optopt = *(p++);
    opt = strchr(optstring, (char)optopt);

    if ( ! opt ) {
        if ( optstring[0] == ':' )
            return MISSING_ARG;
        else
            return UNKNOWN_OPT;
    }

    /* potrzebujemy argumentu */
    if ( opt[1] == ':' ) {
        if ( optind < argc ) {
            optarg = argv[optind];
	} else {
            return MISSING_ARG;
        }
        optind++;
    } else {
        optarg = NULL;
    }
    

    return optopt;
}
