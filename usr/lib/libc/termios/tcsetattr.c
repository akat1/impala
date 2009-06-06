#include <termios.h>
#include <sys/ioctl.h>

struct setattr_args {
    int optact;
    const termios_t *t;
};
typedef struct setattr_args setattr_args_t;

int
tcsetattr(int fd, int optact, const struct termios *termios_p)
{
    setattr_args_t sa;
    sa.t = termios_p;
    sa.optact = optact;
    return ioctl(fd, TCSETS, &sa);
}