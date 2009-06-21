#include <sys/types.h>
#include <sys/vfs.h>
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
    struct mountinfo minfo[N];
    printf("%8s %8s %8s %s\n", 
        "DEV", "TYPE", "OPTIONS", "POINT"
    );
    for( off = 0, n = 0; (n = getmountinfo(off, minfo,N)) && n > 0; off += n ) {
        for (i = 0; i < n; i++) {
            printf("%8s %8s %8s %s\n",
                minfo[i].dev, minfo[i].type, "", minfo[i].mpoint);
        }
    }    
    return 0;
}

