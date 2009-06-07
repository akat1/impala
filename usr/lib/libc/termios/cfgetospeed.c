#include <sys/types.h>
#include <termios.h>


speed_t
cfgetospeed(const struct termios *termios_p)
{
    return B9600;//nie mamy szybko¶ci, wiêc na razie ¶ciema
    //return termios_p->c_ospeed;
}
