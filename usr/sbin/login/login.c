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
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>

static void start_session(void);
static int _login(const char *al);
static struct utsname utsname;



int
_login(const char *autologin)
{
    char login[20];
    char password[20];
    struct termios tio,tio2;
    printf("\n%s-%s/%s at %s\n\n", utsname.sysname, utsname.release,
        utsname.machine, getenv("TTY"));
    if (!autologin) {
        printf("login: ");
        fgets(login, sizeof(login)-1, stdin);
        printf("password: ");
        if (isatty(0)) {
            tcgetattr(0, &tio);
            tio2 = tio;
            UNSET(tio2.c_lflag, ECHO);
            tcsetattr(0, TCSANOW, &tio2);
        }
        fgets(password, sizeof(password)-1, stdin);
        if (isatty(0)) {
            tcsetattr(0, TCSANOW, &tio);
        }
        printf("login failed\n");
        return -1;
    } else {
        printf("autologin: %s\n\n", autologin);
    }
    return 0;
}


void
start_session()
{
    int status;
    pid_t p;
    
    p = fork();
    if (p == -1) {
        fprintf(stderr, "sorry, cannot fork\n");
        sleep(5);
        return;
    }
    if (p == 0) {
        char *argv[] = {
            "-sh",
            NULL
        };
        execve("/bin/sh", argv, environ);
        fprintf(stderr, "cannot start shell\n");
        sleep(5);
        exit(-1);
    }
    waitpid(p, &status, 0);
}

int
main(int argc, char **v)
{
    if (getuid() != 0) {
        fprintf(stderr, "Sorry, login is administrative tool\n");
        return -1;
    }
    uname(&utsname);
    while (1) {
        if (!_login(getenv("AUTOLOGIN")))
            start_session();
    }
    return 0;
}

