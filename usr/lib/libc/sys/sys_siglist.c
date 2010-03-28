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
#include <sys/signal.h>
#include <signal.h>

const char * const sys_siglist[NSIG] = {
"SIGNULL", /* 0 */
"SIGHUP",
"SIGINT",
"SIGQUIT",
"SIGILL",
"SIGTRAP", /* 5 */
"SIGABRT",
"SIGBUS",
"SIGFPE",
"SIGKILL",
"SIGUSR10", /* 10 */
"SIGSEGV",
"SIGUSR2",
"SIGPIPE",
"SIGALRM",
"SIGTERM", /* 15 */
"SIGSTKFLT",
"SIGCHLD",
"SIGCONT",
"SIGSTOP",
"SIGTSTP0", /* 20 */
"SIGTTIN",
"SIGTTOU",
"SIGURG",
"SIGXCPU",
"SIGXFSZ", /* 25 */
"SIGVTALRM",
"SIGPROF",
"SIGWINCH",
"SIGUNUSED",
"SIGUNUSED", /* 30 */
"SIGUNUSED"
};
