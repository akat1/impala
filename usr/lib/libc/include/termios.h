#ifndef __TERMIOS_H
#define __TERMIOS_H

typedef unsigned int XXX;

typedef unsigned int cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define EOF     0
#define VEOL    1
#define VERASE  2
#define VINTR   3
#define VKILL   4
#define VMIN    5
#define VQUIT   6
#define VSTART  7
#define VSTOP   8
#define VSUSP   9
#define VTIME   10
#define NCCS 11

struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
};


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


#define ICRNL 1
#define TCSANOW 1
//#define TCSADRAIN 2
//#define TCSAFLUSH 3


#endif
