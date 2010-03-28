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
#ifndef __TERM_H__
#define __TERM_H__

struct terminfo {
    int a;
};

typedef struct terminfo TERMINAL;

extern char PC;
extern char * UP;
extern char * BC;
extern short ospeed;

//int    del_curterm(TERMINAL *);
//int    putp(const char *);
//int    restartterm(char *, int, int *);
//TERMINAL *set_curterm(TERMINAL *);
//int    setupterm(char *, int, int *);
int    tgetent(char *, const char *);   ///<
int    tgetflag(char *);                ///<
int    tgetnum(char *);                 ///<
char  *tgetstr(char *, char **);        ///<
char  *tgoto(char *, int, int);         ///<
//int    tigetflag(char *); 
//int    tigetnum(char *);
//char  *tigetstr(char *);
//char  *tparm(char *,long, long, long, long, long, long, long, long, long);
int    tputs(const char *, int, int (*)(int));       ///<


#endif
