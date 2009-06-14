#ifndef __SYS_TERMIOS_H
#define __SYS_TERMIOS_H

#define MAX_INPUT 4096

typedef unsigned int cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

//argumenty tcsetattr
#define TCSANOW 1
#define TCSADRAIN 2
#define TCSAFLUSH 3

#define CTRL(x) ((x)&037)
#define EOF    (-1)

//c_cc index:

#define VEOF    0
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
#define NCCS    11

//input flags:

#define BRKINT (1<<0)
#define ICRNL  (1<<1)   //ta flaga potrzebna
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
#define ECHOKE  (ECHOK | ECHOE)
#define ECHONL  (1<<3)
#define ICANON  (1<<4)
#define IEXTEN  (1<<5)
#define ISIG    (1<<6)
#define NOFLSH  (1<<7)
#define TOSTOP  (1<<8)

//speed:

#define B0      0
#define B50     50
#define B75     75
#define B110    110
#define B134    134
#define B150    150
#define B200    200
#define B300    300
#define B600    600
#define B1200   1200
#define B1800   1800
#define B2400   2400
#define B4800   4800
#define B9600   9600
#define B19200  19200
#define B38400  38400

#define _POSIX_VDISABLE '\0'

struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
};
typedef struct termios termios_t;


#ifdef __KERNEL

#include <sys/ascii.h>
#include <sys/clist.h>

typedef int tty_write_t(void* priv, char *ch, size_t size);

struct tty_lowops {
    tty_write_t *tty_write;    ///< zapisuje znak do urz±dzenia
};
typedef struct tty_lowops tty_lowops_t;

struct tty {
    termios_t     t_conf;     ///< ustawienia terminala
    clist_t      *t_inq;      ///< kolejka danych wej¶ciowych
    clist_t      *t_clq;      ///< kolejka aktualnej lini wej¶cia
//    clist_t      *t_outq;     ///< kolejka danych wyj¶ciowych
    tty_lowops_t *t_lowops;   ///< funkcje obs³ugi urz±dzenia komunikacyjnego
    pid_t         t_session;  ///< sesja zwi±zana z terminalem
    pid_t         t_group;    ///< grupa procesów pierwszoplanowych
    void*         t_private;  ///< prywatne dane urz±dzenia komunikacyjnego
    devd_t       *t_dev;      ///< zarejestrowane urz±dzenie terminala
};
typedef struct tty tty_t;

/// tworzy nowe urz±dzenie tty na podstawie urz±dzenia komunikacyjnego
tty_t *tty_create(const char *name, int unit, void *priv, tty_lowops_t *lops);
/// wywo³ywane przez urz±dzenie, gdy dostêpny jest kolejny znak na wej¶ciu
void tty_input(tty_t *tty, int ch);
void tty_output(tty_t *tty, char ch);




#endif



#endif
