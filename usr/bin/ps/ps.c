#include <sys/types.h>
#include <sys/proc.h>
#include <stdio.h>

int
main(int argc, char **v)
{
    enum {  
        N = 20
    };
    int i;
    int n;
    int off;
    struct procinfo pinfo[N];
    printf("%5s %5s %5s %5s %5s %7s   %s\n", 
        "PID", "PPID", "NICE", "PRI", "THRS", "TTY", "IMAGE"
    );
    for( off = 0, n = 0; (n = getprocinfo(off, pinfo,N)) && n > 0; off += n ) {
        for (i = 0; i < n; i++) {
            printf("%5u %5u %5u %5u %5u %7s   %s\n",
                pinfo[i].pid, pinfo[i].ppid, pinfo[i].nice,
                pinfo[i].pri, pinfo[i].threads,
                pinfo[i].tty, pinfo[i].cmd);
        }
    }    
    return 0;
}

