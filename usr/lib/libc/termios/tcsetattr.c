#include <termios.h>
#include <sys/ioctl.h>


int
tcsetattr(int fd, int optact, const struct termios *termios_p)
{
    if(optact == TCSANOW)
        return ioctl(fd, TCSETS, termios_p);
    else
        return -1;
}