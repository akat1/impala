#ifndef __TERMIOS_H
#define __TERMIOS_H

#include <sys/termios.h>


int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions,
                 const struct termios *termios_p);
// int tcsendbreak(int fd, int duration);
// int tcdrain(int fd);
// int tcflush(int fd, int queue_selector);
// int tcflow(int fd, int action);
void cfmakeraw(struct termios *termios_p);
// speed_t cfgetispeed(const struct termios *termios_p);
// speed_t cfgetospeed(const struct termios *termios_p);
// int cfsetispeed(struct termios *termios_p, speed_t speed);
// int cfsetospeed(struct termios *termios_p, speed_t speed);
// int cfsetspeed(struct termios *termios_p, speed_t speed);





#endif
