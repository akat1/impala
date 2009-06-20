#include <sys/types.h>
#include <string.h>
#include <term.h>

#define ERR 1

int
tgetent(char *bp, const char *name)
{
    return ERR;
}

int
tgetflag(char id[2])
{
    return ERR;
}

int
tgetnum(char id[2])
{
    return ERR;
}

char *
tgetstr(char id[2], char **area)
{
    return NULL;
}

char *
tgoto(char *cap, int col, int row)
{
    return NULL;
}

int tputs(const char *str, int affcnt, int (*putfunc)(int))
{
    return 0;
}