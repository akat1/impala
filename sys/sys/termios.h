#ifndef __SYS_TERMIOS_H
#define __SYS_TERMIOS_H


#define ICRNL 1
#define TCSANOW 1
//#define TCSADRAIN 2
//#define TCSAFLUSH 3

#define VEOF     0
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

typedef unsigned int cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;


struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
};



#ifdef __KERNEL



#endif



#endif
