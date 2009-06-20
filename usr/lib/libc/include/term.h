#ifndef __TERM_H__
#define __TERM_H__

struct terminfo {
    int a;
};

typedef struct terminfo TERMINAL;

int    del_curterm(TERMINAL *);
int    putp(const char *);
int    restartterm(char *, int, int *);
TERMINAL *set_curterm(TERMINAL *);
int    setupterm(char *, int, int *);
int    tgetent(char *, const char *);   ///<
int    tgetflag(char *);                ///<
int    tgetnum(char *);                 ///<
char  *tgetstr(char *, char **);        ///<
char  *tgoto(char *, int, int);         ///<
int    tigetflag(char *); 
int    tigetnum(char *);
char  *tigetstr(char *);
char  *tparm(char *,long, long, long, long, long, long, long, long, long);
int    tputs(const char *, int, int (*)(int));       ///<


#endif
