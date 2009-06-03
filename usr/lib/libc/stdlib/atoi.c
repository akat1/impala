#include <sys/types.h>
#include <stdlib.h>

int
atoi(const char *nptr)
{
	return strtol(nptr, (char **)NULL, 10);
}
