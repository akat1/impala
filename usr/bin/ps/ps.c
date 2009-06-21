#include <sys/types.h>
#include <sys/proc.h>



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
    printf("%5s %5s %5s %10s %s\n", 
        "PID", "PPID", "THRS", "TTY", "CMD"
    );
    for( off = 0, n = 0; (n = getprocinfo(off, pinfo,N)) && n > 0; off += n ) {
        for (i = 0; i < n; i++) {
            printf("%5u %5u %5u %10s %s\n",
                pinfo[i].pid, pinfo[i].ppid, pinfo[i].threads,
                pinfo[i].tty, pinfo[i].cmd);
        }
    }    
    return 0;
}

