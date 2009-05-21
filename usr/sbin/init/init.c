#include <sys/types.h>
#include <unistd.h>
#include <string.h>

const char data[] = "Hello, World\n";

int
main(int argc, char **argv)
{
    write(0, data, strlen(data));
    return 0;
}
