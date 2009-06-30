#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>

char *optarg;
int optind = 1, opterr, optopt, optreset;

#define UNKNOWN_OPT ((int)'?')
#define MISSING_ARG ((int)':')

int
getopt(int argc, char * const argv[], const char *optstring)
{
    static char *p = "";
    char *opt;

    /* jeste¶my na koñcu */
    if ( *p == '\0' || optreset ) {
        if ( optind >= argc ) {
            p = "";
            return -1;
        }

        p = argv[optind++];

        /* oczekujemy opcji */
        if ( p[0] != '-' ) {
            p = "";
            return -1;
        }

        /* je¿eli dostali¶my --, to koniec */
        if ( p[0] == '-' && p[1] == '-' ) {
            p = "";
            return -1;
        }

        /* dostali¶my jak±¶ opcje */
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
