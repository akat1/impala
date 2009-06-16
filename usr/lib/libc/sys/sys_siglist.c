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
