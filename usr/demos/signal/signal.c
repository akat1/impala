#include <signal.h>


void test(int sig);
void test2(int sig);

void test(int sig)
{
	printf("I'VE RECEIVED SIGHUP!\n");
    return;
}

void test2(int sig)
{
	printf("I'VE RECEIVED SIGUSR1!\n");
    return;
}

int
main()
{
    printf("handlers..\n");
	signal(SIGHUP, test);
	signal(SIGUSR1, test2);

    raise(SIGHUP);
    raise(SIGUSR1);

	return 0;
}
