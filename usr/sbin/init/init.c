#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>



char *envp[] = {
    "TERMCAP=vt100-8025|vt100 emulator with ANSI colors 80x25:\\\n"
        ":pa#64:Co#8:AF=\\E[3%dm:AB=\\E[4%dm:op=\\E[39;49m\\E[m:\\\n"
        ":do=2\\E[B:co#80:li#24:cl=50\\E[H\\E[J:sf=2*\\ED:\\\n"
        ":le=^H:bs:am:cm=5\\E[%i%d;%dH:nd=2\\E[C:up=2\\E[A:\\\n"
        ":ce=3\\E[K:cd=50\\E[J:so=2\\E[7m:se=2\\E[m:us=2\\E[4m:ue=2\\E[m:\\\n"
        ":md=2\\E[1m:mr=2\\E[7m:mb=2\\E[5m:me=2\\E[m:\\\n"
        ":is=\\E>\\E[?1;3;4;5l\\E[?7;8h\\E[1;24r\\E[24;1H:\\\n"
        ":if=/usr/share/tabset/vt100:nw=2\\EE:ho=\\E[H:\\\n"
        ":as=2\\E(0:ae=2\\E(B:\\n"
        ":ac=``aaffggjjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||:\\\n"
        ":rs=\\E>\\E[?1;3;4;5l\\E[?7;8h:ks=\\E[?1h\\E=:ke=\\E[?1l\\E>:\\\n"
        ":ku=\\EOA:kd=\\EOB:kr=\\EOC:kl=\\EOD:kb=\177:\\\n"
        ":k0=\\EOy:k1=\\EOP:k2=\\EOQ:k3=\\EOR:k4=\\EOS:k5=\\EOt:\\\n"
        ":k6=\\EOu:k7=\\EOv:k8=\\EOl:k9=\\EOw:k;=\\EOx:@8=\\EOM:\\\n"
        ":K1=\\EOq:K2=\\EOr:K3=\\EOs:K4=\\EOp:K5=\\EOn:pt:sr=2*\\EM:xn:\\\n"
        ":sc=2\\E7:rc=2\\E8:cs=5\\E[%i%d;%dr:UP=2\\E[%dA:DO=2\\E[%dB:RI=2\\E[%dC:\\\n"
        ":LE=2\\E[%dD:ct=2\\E[3g:st=2\\EH:ta=^I:ms:bl=^G:cr=^M:eo:it#8:\\\n"
        ":RA=\\E[?7l:SA=\\E[?7h:",
    "TERM=vt100-8025",
    NULL
};

int
main(int argc, char **v)
{
    char *argv[] = {
        "sh",
        "/etc/rc.start",
        NULL
    };
    pid_t p;
#ifdef __Impala__
    setsid();
    open("/dev/ttyv1", O_RDONLY);   //stdin
    open("/dev/ttyv1", O_WRONLY);   //stdout
    open("/dev/ttyv1", O_WRONLY);   //stderr
#endif

    if (getpid() != 1) {
        return 0;
    }
    p = fork();
    if (p == -1) {
        fprintf(stderr, "cannot fork\n");
        while (1);
    } else
    if (p == 0) {
        execve("/bin/sh", argv, envp);
    }
    for (;;);
    return 0;
}
