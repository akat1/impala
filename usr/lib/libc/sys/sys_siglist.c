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
"SIGIOT",
"SIGBUS",
"SIGFPE",
"SIGKILL", /* 10 */
"SIGUSR10",
"SIGSEGV",
"SIGUSR2",
"SIGPIPE",
"SIGALRM", /* 15 */
"SIGTERM",
"SIGSTKFLT",
"SIGCHLD",
"SIGCONT",
"SIGSTOP", /* 20 */
"SIGTSTP0",
"SIGTTIN",
"SIGTTOU",
"SIGURG",
"SIGXCPU", /* 25 */
"SIGXFSZ",
"SIGVTALRM",
"SIGPROF",
"SIGWINCH",
"SIGUNUSED", /* 30 */
"SIGUNUSED"
};
