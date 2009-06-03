#ifndef __SYS_TERMIOS_H
#define __SYS_TERMIOS_H


#define ICRNL 1
#define TCSANOW 1
//#define TCSADRAIN 2
//#define TCSAFLUSH 3

//c_cc index:

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

//input flags:

#define BRKINT (1<<0)
#define ICRNL  (1<<1)
#define IGNBRK (1<<2)
#define IGNCR  (1<<3)
#define IGNPAR (1<<4)
#define INLCR  (1<<5)
#define INPCK  (1<<6)
#define ISTRIP (1<<7)
#define IXANY  (1<<8)
#define IXOFF  (1<<9)
#define IXON   (1<<10)
#define PARMRK (1<<11)

//output flags:

#define OPOST   (1<<0)
#define ONLCR   (1<<1)
#define OCRNL   (1<<2)
#define ONOCR   (1<<3)
#define ONLRET  (1<<4)
#define OFILL   (1<<5)
#define OFDEL   (1<<6)
#define NLDLY   (1<<7)
#define NL0     (1<<8)
#define NL1     (1<<9)
#define CRDLY   (1<<10)
#define CR0     (1<<11)
#define CR1     (1<<12)
#define CR2     (1<<13)
#define CR3     (1<<14)
#define TABDLY  (1<<15)
#define TAB0    (1<<16)
#define TAB1    (1<<17)
#define TAB2    (1<<18)
#define TAB3    (1<<19)
#define BSDLY   (1<<20)
#define BS0     (1<<21)
#define BS1     (1<<22)
#define VTDLY   (1<<23)
#define VT0     (1<<24)
#define VT1     (1<<25)
#define FFDLY   (1<<26)
#define FF0     (1<<27)
#define FF1     (1<<28)


//control flags:

#define CLOCAL  (1<<0)
#define CREAD   (1<<1)
#define CSIZE   (1<<2)
#define CS5     (1<<3)
#define CS6     (1<<4)
#define CS7     (1<<5)
#define CS8     (1<<6)
#define CSTOPB  (1<<7)
#define HUPCL   (1<<8)
#define PARENB  (1<<9)
#define PARODD  (1<<10)

//local flags:

#define ECHO    (1<<0)
#define ECHOE   (1<<1)
#define ECHOK   (1<<2)
#define ECHONL  (1<<3)
#define ICANON  (1<<4)
#define IEXTEN  (1<<5)
#define ISIG    (1<<6)
#define NOFLSH  (1<<7)
#define TOSTOP  (1<<8)


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
