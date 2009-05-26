/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include <fs/devfs/devfs.h>
#include <sys/types.h>
#include <sys/console.h>
#include <sys/kmem.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/thread.h>
#include <sys/errno.h>
#include <sys/utils.h>
#include <sys/uio.h>
#include <machine/video.h>
#include <machine/interrupt.h>
#include <machine/pckbd.h>

enum {
    DEFAULT_FG = COLOR_BRIGHTGRAY,
    DEFAULT_BG = COLOR_BLACK,
    HIDDEN = COLOR_DARKGRAY,
    CONSOLE_ATTR= COLOR_WHITE
};

#define CONSOLE_ATTR_CODE "\033[s\033[1;30;40m"

enum {
    PARSER_MAX_ATTRS = 10
};

enum {
    VTTY_MAX = 3
};


typedef struct vt_parser vt_parser_t;
struct vt_parser {
    int     state;
    int     value;
    int     attr[PARSER_MAX_ATTRS];
    int     attr_i;
};


typedef struct vtty vtty_t;
struct vtty {
    int             num;
    int             bold;
    int             sattr;
    int             sattr2;
    int             cx;
    int             cy;
    textscreen_t    screen;
    vt_parser_t     parser;
    mutex_t         mtx;
    devd_t         *dev;
};


static void vtty_put(vtty_t *t, char c);
static void vtty_putstr(vtty_t *t, const char *c);
void vtty_data_out(vtty_t *vt, const char *c, int n);

static int vt_parser_put(vt_parser_t *vtprs, char c);
static void vt_parser_reset(vt_parser_t *vtprs);
static void vtty_code(vtty_t *vt, int c);



enum {
    PARSER_ERROR    = -1,
    PARSER_CONT     = -2,
};

static d_open_t ttyv_open;
static d_close_t ttyv_close;
static d_write_t ttyv_write;
static d_read_t ttyv_read;
static d_close_t ttyv_close;
static d_ioctl_t ttyv_ioctl;

static d_open_t cons_open;
static d_close_t cons_close;
static d_write_t cons_write;
static d_read_t cons_read;
static d_close_t cons_close;
static d_ioctl_t cons_ioctl;

static devsw_t ttyvsw = {
    ttyv_open,
    ttyv_close,
    ttyv_ioctl,
    ttyv_read,
    ttyv_write,
    nostrategy,
    "ttyv"
};

static devsw_t conssw = {
    cons_open,
    cons_close,
    cons_ioctl,
    cons_read,
    cons_write,
    nostrategy,
    "console"
};


static vtty_t vttys[VTTY_MAX];
static vtty_t *current_vtty = NULL;
static devd_t *consdev;

// Odwzorowanie kolorów czcionki z sekwencji steruj±cych na kolory VGA
static int tty_fgattr_map[10] = {
    TS_FG(DEFAULT_FG),
    TS_FG(COLOR_RED),
    TS_FG(COLOR_GREEN),
    TS_FG(COLOR_BROWN),
    TS_FG(COLOR_BLUE),
    TS_FG(COLOR_MAGENTA),
    TS_FG(COLOR_CYAN),
    TS_FG(COLOR_BRIGHTGRAY),
    TS_FG(COLOR_BRIGHTGRAY),
    TS_FG(COLOR_BRIGHTGRAY)
};

// Odwzorowanie kolorów t³a z sekwencji steruj±cych na kolory VGA
static int tty_bgattr_map[10] = {
    TS_BG(DEFAULT_BG),
    TS_BG(COLOR_RED),
    TS_BG(COLOR_GREEN),
    TS_BG(COLOR_BROWN),
    TS_BG(COLOR_BLUE),
    TS_BG(COLOR_MAGENTA),
    TS_BG(COLOR_CYAN),
    TS_BG(COLOR_BRIGHTGRAY),
    TS_BG(COLOR_BLACK),
    TS_BG(COLOR_BLACK)
};


/*========================================================================
 * Konsola
 */

void
cons_init()
{
    consdev = devd_create(&conssw, -1, NULL);
    devfs_register(consdev->name, consdev, 0, 0, 0777);
    for (int i = 0; i < VTTY_MAX; i++) {
        mutex_init(&vttys[i].mtx, MUTEX_NORMAL);
        vttys[i].dev = devd_create(&ttyvsw, i, &vttys[i]);
        devfs_register(vttys[i].dev->name, vttys[i].dev, 0, 0, 0777);
        vttys[i].sattr = COLOR_BRIGHTGRAY;
        if (i > 0) {
            textscreen_clone(&vttys[i].screen);
            textscreen_clear(&vttys[i].screen);
        }
    }
    textscreen_clone(&vttys[0].screen);
    textscreen_switch(&vttys[0].screen);
    current_vtty = &vttys[0];
}

#define isprint(c) ( 31 < c && c < 127 )
void
cons_output(int t, const char *c)
{
    if (current_vtty) {
        if (t != CONS_TTY) {
            char buf[SPRINTF_BUFSIZE];
            char *ptr = str_cpy(buf,CONSOLE_ATTR_CODE);
            ptr = str_cat(ptr, c);
            str_cat(ptr, "\033[u");
            vtty_putstr(current_vtty, buf);
        } else {
            vtty_putstr(current_vtty, c);
        }
    } else {
        // Je¿eli obs³uga terminali nie jest jeszcze zainicjalizowana
        // to nadajemy rêcznie na ekran
        for (; *c; c++) {
            if (isprint(*c)) textscreen_put(NULL, *c, CONSOLE_ATTR);
            if (*c == '\n') textscreen_next_line(NULL);
        }
    }
}


/*========================================================================
 * Obs³uga wirtualnych terminali w emulacji VT100
 */

/*
 * Sekwencje steruj±cy terminalu VT100.
 * Wszystkie zaczynaj± siê od znaku ESCAPE, klamrami {}
 * s± oznaczone zmienne. Zapis {X=y} oznacza, ¿e parametr
 * X mo¿e nie zostaæ podany, i wtedy przyjmuje domy¶ln± warto¶æ y.
 * Opis kodów dostêpny na stronie (kody poni¿ej s± w tej samej kolejno¶æi)
 *      http://www.termsys.demon.co.uk/vtansi.htm
 *
 * Dodatkowe oznaczenia:
 *  #   - kod jest rozpoznawany przez sterownik
 *  @   - kod jest rozpoznawany i obs³ugiwany przez sterownik
 */
enum {
    ESC_RESET,      // c (@)
    ESC_LWRAPON,    // [7h
    ESC_LWRAPOFF,   // [71
    ESC_G0,         // ( (#)
    ESC_G1,         // ) (#)
    ESC_CURHOME,    // [{ROW=0};{COL=0}H (@)
    ESC_CURUP,      // [{COUNT=1}A (@)
    ESC_CURDOWN,    // [{COUNT=1}B (@)
    ESC_CURFORW,    // [{COUNT=1}C (@)
    ESC_CURBACK,    // [{COUNT=1}D (@)
    ESC_CURFORCE,   // [{ROW=0};{COL=0}f (@)
    ESC_CURSAVE,    // [s (@)
    ESC_CURLOAD,    // [u (@)
    ESC_CURSAVEA,   // 7 (@)
    ESC_CURLOADA,   // 8 (@)
    ESC_SCROLLE,    // [r (#)
    ESC_SCROLL,     // [{start};{end}r (#)
    ESC_SCROLLD,    // D (#)
    ESC_SCROLLU,    // M (#)
    ESC_TABSET,     // H (#)
    ESC_TABCLR,     // [g (#)
    ESC_TABCLRA,    // [3g
    ESC_ERASEE,     // [K (#)
    ESC_ERASEB,     // [1K
    ESC_ERASEL,     // [2K
    ESC_ERASED,     // [J (#)
    ESC_ERASEU,     // [1J (#)
    ESC_ERASES,     // [2J (@)
    ESC_PRINTS,     // [i (#)
    ESC_PRINTL,     // [1i (#)
    ESC_LOGS,       // [4i (#)
    ESC_LOGE,       // [7i (#)
    ESC_DEFKEY,     // [{key};"{string}"p
    ESC_ATTR,       // [{attr1};...;{attrn}m (@)
};


void
vtty_putstr(vtty_t *vt, const char *c)
{
    vtty_data_out(vt, c, str_len(c));
}

void
vtty_data_out(vtty_t *vt, const char *c, int n)
{
    enum {
        CODE_ESC = 033
    };
    bool escape = FALSE;
    int X = spltty();
    mutex_lock(&vt->mtx);
    for (; n; c++, n--) {
        if (escape) {
            int code = vt_parser_put(&vt->parser, *c);
            if (code == PARSER_ERROR) {
                escape = FALSE;
            } else
            if (code != PARSER_CONT) {
                vtty_code(vt, code);
                escape = FALSE;
            }
        } else {
            if (*c == CODE_ESC) {
                escape = TRUE;
                vt_parser_reset(&vt->parser);
            } else {
                vtty_put(vt, *c);
            }
        }
    }
    mutex_unlock(&vt->mtx);
    splx(X);
}


void
vtty_put(vtty_t *vt, char c)
{
    if (isprint(c)) {
        textscreen_put(&vt->screen, c, vt->sattr);
    } else
    if (c == '\n') {
        textscreen_next_line(&vt->screen);
    } else {
        textscreen_put(&vt->screen, '?', vt->sattr);
    }
}

void
vtty_code(vtty_t *vt, int c)
{
    int cx,cy;
    textscreen_get_cursor(&vt->screen, &cx, &cy);
    switch (c) {
        case ESC_CURHOME:
            cx = vt->parser.attr[0];
            cy = vt->parser.attr[1];
            textscreen_update_cursor(&vt->screen, cx, cy);
            break;
        case ESC_CURSAVEA:
            vt->cx = cx;
            vt->cy = cy;
        case ESC_CURSAVE:
            vt->sattr2 = vt->sattr;
            break;
        case ESC_CURLOADA:
            textscreen_update_cursor(&vt->screen, vt->cx, vt->cy);
        case ESC_CURLOAD:
            vt->sattr = vt->sattr2;
            break;
        case ESC_CURUP:
            cy -= vt->parser.attr[0];
            textscreen_update_cursor(&vt->screen, cx, cy);
            break;
        case ESC_CURDOWN:
            cy += vt->parser.attr[0];
            textscreen_update_cursor(&vt->screen, cx, cy);
            break;
        case ESC_CURFORW:
            cx += vt->parser.attr[0];
            textscreen_update_cursor(&vt->screen, cx, cy);
            break;
        case ESC_CURBACK:
            cx -= vt->parser.attr[0];
            textscreen_update_cursor(&vt->screen, cx, cy);
            break;
        case ESC_RESET:
            textscreen_clear(&vt->screen);
            vt->sattr = TS_FG(DEFAULT_FG)|TS_BG(DEFAULT_BG);
            break;
        case ESC_ERASES:
            textscreen_clear(&vt->screen);
            break;
        case ESC_ATTR:
            for (int i = 0; i <= vt->parser.attr_i; i++) {
                int a = vt->parser.attr[i];
                if (a < 10) {
                    switch (a) {
                        case 0:
                            vt->sattr = TS_FG(DEFAULT_FG)|TS_BG(DEFAULT_BG);
                            break;
                        case 1:
                            vt->sattr = TS_BOLD(vt->sattr);
                            break;
                        case 5: // mruganie
                            break;
                        case 7:
                            vt->sattr = TS_BG(_TS_FG(vt->sattr)) |
                                TS_FG(_TS_BG(vt->sattr));
                            break;
                    }
                } else
                if (30 <= a && a < 40) {
                    vt->sattr = _TS_BG(vt->sattr) | _TS_BOLD(vt->sattr)
                        | tty_fgattr_map[a-30];
                } else
                if (40 <= a && a < 50) {
                    vt->sattr = _TS_FG(vt->sattr) | _TS_BOLD(vt->sattr)
                        | tty_bgattr_map[a-40];
                }
            }
            break;
    }
}



/*========================================================================
 * Plik urz±dzenia: /dev/console
 */

int
cons_open(devd_t *d, int flags)
{
    return -ENOTSUP;
}

int
cons_read(devd_t *d, uio_t *u)
{
    return -ENOTSUP;
}

int
cons_write(devd_t *d, uio_t *u)
{
    return -ENOTSUP;
}

int
cons_close(devd_t *d)
{
    return -ENOTSUP;
}

int
cons_ioctl(devd_t *d, int cmd, uintptr_t param)
{
    return -ENOTSUP;
}

/*========================================================================
 * Plik urz±dzenia: /dev/ttyvXX
 */

int
ttyv_open(devd_t *d, int flags)
{
    return 0;
}

int
ttyv_read(devd_t *d, uio_t *u)
{
//    vtty_t *vtty = d->priv;
    //tymczasowo, chyba na potrzeby demka 
    char c;
    char BUF[512];
    char *b = BUF;
    while((c = pckbd_get_char())!=-1 && b<BUF+511)
        *(b++) = c;
    *b = 0;
    int n =  uio_move(BUF, b-BUF, u);
    return n;
}

int
ttyv_write(devd_t *d, uio_t *u)
{
    vtty_t *vtty = d->priv;
    char BUF[512];
    int n =  uio_move(BUF, MIN(512, u->size), u);
    vtty_data_out(vtty, BUF, MIN(512, u->size));
    return n;
}

int
ttyv_ioctl(devd_t *d, int cmd, uintptr_t param)
{
    return -ENOTSUP;
}

int
ttyv_close(devd_t *d)
{
    return -ENOTSUP;
}


/*========================================================================
 * Implementacja parsera skanuj±cego sekwencje steruj±ce VT100
 */


enum {
    P_DUMMY,
    P_FIRST,         // czekamy na pierwszy znak
    P_LONG,          // czekamy na znaczki po '['
    P_LONG_ATTR      // czekamy na atrybut po '['

};

void
vt_parser_reset(vt_parser_t *vtprs)
{
    mem_zero(vtprs, sizeof(*vtprs));
    vtprs->state = P_FIRST;
}

/*
 * Zastanawiam siê czy nie przepisaæ tego na jaki¶ DFA.
 */
int
vt_parser_put(vt_parser_t *vtprs, char c)
{
    int ret = PARSER_CONT;
    int nexts = P_DUMMY;
    if(vtprs->state == P_FIRST) {
        switch (c) {
            case 'c':
                ret = ESC_RESET;
                break;
            case '(':
                ret = ESC_G0;
                break;
            case ')':
                ret = ESC_G1;
                break;
            case '7':
                ret = ESC_CURSAVEA;
                break;
            case '8':
                ret = ESC_CURLOADA;
                break;
            case 'D':
                ret = ESC_SCROLLD;
                break;
            case 'M':
                ret = ESC_SCROLLU;
                break;
            case 'H':
                ret = ESC_TABSET;
                break;
            case '[':
                nexts = P_LONG;
                break;
            default:
                ret = PARSER_ERROR;
                break;
        }
    } else
    if (vtprs->state == P_LONG) {
        if ( '0' <= c && c <= '9') {
            vtprs->attr[0] = c - '0';
            vtprs->attr_i = 0;
            nexts = P_LONG_ATTR;
        } else
        switch (c) {
            case 's':
                ret = ESC_CURSAVE;
                break;
            case 'u':
                ret = ESC_CURLOAD;
                break;
            case 'r':
                ret = ESC_SCROLLE;
                break;
            case 'g':
                ret = ESC_TABCLR;
                break;
            case 'K':
                ret = ESC_ERASEE;
                break;
            case 'J':
                ret = ESC_ERASED;
                break;
            case 'i':
                ret = ESC_PRINTS;
                break;
            case 'f':
            case 'H':
                ret = ESC_CURHOME;
                break;
            case 'A':
                ret = ESC_CURUP;
                vtprs->attr[0] = 1;
                break;
            case 'B':
                ret = ESC_CURDOWN;
                vtprs->attr[0] = 1;
                break;
            case 'C':
                ret = ESC_CURFORW;
                vtprs->attr[0] = 1;
                break;
            case 'D':
                ret = ESC_CURBACK;
                vtprs->attr[0] = 1;
                break;
            default:
                ret = PARSER_ERROR;
                break;
        }
    } else
    if (vtprs->state == P_LONG_ATTR) {
        if ('0' <= c && c <= '9') {
            vtprs->attr[vtprs->attr_i] *= 10;
            vtprs->attr[vtprs->attr_i] += c - '0';
            nexts = P_LONG_ATTR;
        } else
        switch (c) {
            case ';':
                nexts = P_LONG_ATTR;
                vtprs->attr_i++;
                vtprs->attr[vtprs->attr_i] = 0;
                break;
            case 'm':
                ret = ESC_ATTR;
                break;
            case 'f':
            case 'H':
                ret = ESC_CURHOME;
                break;
            case 'A':
                ret = ESC_CURUP;
                break;
            case 'B':
                ret = ESC_CURDOWN;
                break;
            case 'C':
                ret = ESC_CURFORW;
                break;
            case 'D':
                ret = ESC_CURBACK;
                break;
            case 'r':
                ret = ESC_SCROLL;
                break;
            case 'J':
                switch (vtprs->attr[0]) {
                    case 1:
                        ret = ESC_ERASEU;
                        break;
                    case 2:
                        ret = ESC_ERASES;
                        break;
                    default:
                        ret = PARSER_ERROR;
                        break;
                }
                break;
            case 'i':
                switch (vtprs->attr[0]) {
                    case 1:
                        ret = ESC_PRINTL;
                        break;
                    case 4:
                        ret = ESC_LOGS;
                        break;
                    case 5:
                        ret = ESC_LOGE;
                        break;
                }
                break;
            default:
                ret = PARSER_ERROR;
                break;
        }
    }
    if (vtprs->attr_i == PARSER_MAX_ATTRS) {
        vtprs->attr_i = 0;
        ret = PARSER_ERROR;
    }
    vtprs->state = nexts;
    return ret;
}
