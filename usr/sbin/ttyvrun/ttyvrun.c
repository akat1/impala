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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <sys/thread.h>

void tmain(void);
void inputline(char *s);

int
main(int argc, char **v)
{
    if (argc < 3) {
        return -1;
    }
    close(0);
    close(1);
    close(2);
    // jeżeli jesteśmy leaderem grupy, to nie możemy zrobić setsid -> trik ;)
    pid_t p = fork();
    if (p) exit(0); 
    setsid();
    open(v[1], O_RDONLY);
    open(v[1], O_WRONLY);
    open(v[1], O_WRONLY);
    v+=2;
    execve(v[0], v , environ);
    return 0;
}
