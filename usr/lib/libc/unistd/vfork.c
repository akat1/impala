#include <unistd.h>

pid_t 
vfork(void)
{
    return fork();  // a co... nie taki by� cel, ale nadal "compliant" ;p
}
