#include <stdlib.h>

void
abort(void)
{
    //raise(SIGABORT);
    exit(EXIT_FAILURE);
}