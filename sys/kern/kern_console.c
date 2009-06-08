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
#include <sys/ctty.h>
#include <sys/types.h>
#include <sys/console.h>
#include <sys/termios.h>
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
#include <sys/ascii.h>

enum {
    DEFAULT_FG = COLOR_BRIGHTGRAY,
    DEFAULT_BG = COLOR_BLACK,
    HIDDEN = COLOR_DARKGRAY,
    CONSOLE_ATTR= COLOR_WHITE
};

#define CONSOLE_ATTR_CODE "\0337\033[1;30;40m"

enum {
    PARSER_MAX_ATTRS = 10
};

enum {
    VCONS_MAX = 3
};


typedef struct vc_parser vc_parser_t;
struct vc_parser {
    int     state;
    int     value;
    int     attr[PARSER_MAX_ATTRS];
    int     attr_i;
};


typedef struct vconsole vconsole_t;

/**
 * Struktura reprezentuj±ca wirtualn± konsolê - wirtualny zestaw
 * klawiatura - monitor. Prawdziwe urz±dzenia podpiête s± pod aktualn± wirtualn±
 * konsolê - current_vcons
 */

struct vconsole {
    int             num;
    int             bold;
    int             sattr;
    int             sattr2;
    int             cx;
    int             cy;
    int             mode;
    bool            escape;
    textscreen_t    screen;
    vc_parser_t     parser;
    mutex_t         mtx;
//    devd_t         *dev;
    tty_t          *tty;    ///< urz±dzenie terminalowe tej konsoli wirtualnej
};

#define CONS_MODE_AWRAP   1
#define CONS_MODE_ORIGIN  2
#define CONS_MODE_SCREEN  4
#define CONS_MODE_NEWLINE 8

static void vcons_input_char(vconsole_t *vc, int ch);
static void vcons_input_string(vconsole_t *vc, const char* str);
static void vcons_put(vconsole_t *t, char c);
static void vcons_putstr(vconsole_t *t, const char *c);
static void reset_mode(vconsole_t *vc, int m);
static void set_mode(vconsole_t *vc, int m);
void vcons_data_out(vconsole_t *vc, const char *c, int n);

static int vc_parser_put(vc_parser_t *vcprs, char c);
static void vc_parser_reset(vc_parser_t *vcprs);
static void vc_parser_resetE(vc_parser_t *vcprs);
static void vcons_code(vconsole_t *vc, int c);




enum {
    PARSER_ERROR    = -1,
    PARSER_CONT     = -2,
};


static vconsole_t vcons[VCONS_MAX];
static vconsole_t *current_vcons = NULL;
static tty_t  *current_vcons_tty = NULL;
//static devd_t *consttydev;

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

tty_write_t vcons_write;

int vcons_write(void *t, char *b, size_t s)
{
    vcons_data_out((vconsole_t*)t, b, s);
    return s;
}


tty_lowops_t vcons_lowop = {
    .tty_write = vcons_write,
};

// tty_lowops_t cons_lowop = {
//     .tty_write = cons_write,
// };

void
cons_init()
{
    //tty_create("console", -1, &console_data, &cons_lowop);
//    devfs_register(consdev->name, consdev, 0, 0, 0777);

    ctty_create(); //tworzymy /dev/tty
    for (int i = 0; i < VCONS_MAX; i++) {
        mutex_init(&vcons[i].mtx, MUTEX_NORMAL);
        vcons[i].tty = tty_create("ttyv%i", i+1, &vcons[i], &vcons_lowop);
        vcons[i].sattr = COLOR_BRIGHTGRAY;
        vcons[i].escape = FALSE;
        vcons[i].mode = CONS_MODE_AWRAP | CONS_MODE_NEWLINE;
        textscreen_init_tab(&vcons[i].screen);
        if (i > 0) {
            textscreen_clone(&vcons[i].screen);
            textscreen_clear(&vcons[i].screen);
        }
    }
    textscreen_clone(&vcons[0].screen);
    textscreen_switch(&vcons[0].screen);
    current_vcons = &vcons[0];
    current_vcons_tty = current_vcons->tty;
}

#define isprint(c) ( 31 < c && c < 127 )
void
cons_output(int t, const char *c)
{
    if (current_vcons) {
        if (t != CONS_TTY) {
            char buf[SPRINTF_BUFSIZE];
            char *ptr = str_cpy(buf,CONSOLE_ATTR_CODE);
            ptr = str_cat(ptr, c);
            str_cat(ptr, "\0338");
            vcons_putstr(current_vcons, buf);
        } else {
            vcons_putstr(current_vcons, c);
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

//wywo³ywane póki co ze sterownika klawiatury
void cons_input_char(int ch)
{
    if(current_vcons) {
        vcons_input_char(current_vcons, ch);
    }
}

void cons_input_string(const char *str)
{
    if(current_vcons) {
        vcons_input_string(current_vcons, str);
    }
}


/*========================================================================
 * Obs³uga wirtualnych terminali w emulacji VT100
 */

/*
 * Sekwencje steruj±ce terminalu VT100.
 * Wszystkie zaczynaj± siê od znaku ESCAPE, klamrami {}
 * s± oznaczone zmienne. Zapis {X=y} oznacza, ¿e parametr
 * X mo¿e nie zostaæ podany, i wtedy przyjmuje domy¶ln± warto¶æ y.
 * Opis kodów dostêpny na stronie (kody poni¿ej s± w tej samej kolejno¶ci)
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
    ESC_SCROLL,     // [{start};{end}r (#)
    ESC_MOVED,      // D (#)
    ESC_MOVEU,      // M (#)
    ESC_TABSET,     // H (#)
    ESC_TABCLR,     // [{attr}g (#)
    ESC_DECALN,     // #8 (@)
    ESC_ERASEE,     // [K (@)
    ESC_ERASEB,     // [1K (@)
    ESC_ERASEL,     // [2K (@)
    ESC_ERASED,     // [J (@)
    ESC_ERASEU,     // [1J (@)
    ESC_ERASES,     // [2J (@)
    ESC_PRINTS,     // [i (#)
    ESC_PRINTL,     // [1i (#)
    ESC_LOGS,       // [4i (#)
    ESC_LOGE,       // [7i (#)
    ESC_SETMODE,    // [{};{}h
    ESC_RESMODE,    // [{};{}l
    ESC_NEXTL,      // E        (@)
    ESC_DEFKEY,     // [{key};"{string}"p
    ESC_ATTR,       // [{attr1};...;{attrn}m (@)
};


void
vcons_input_char(vconsole_t *vc, int ch)
{
    tty_input(vc->tty, ch);
}

void
vcons_input_string(vconsole_t *vc, const char* str)
{
    tty_t *tty = vc->tty;
    for(const char *c=str; *c; c++)
        tty_input(tty, *c);
}


void
vcons_putstr(vconsole_t *vc, const char *c)
{
    vcons_data_out(vc, c, str_len(c));
}

void
vcons_data_out(vconsole_t *vc, const char *cc, int n)
{
    enum {
        CODE_ESC = 033
    };
    int X = spltty();
    mutex_lock(&vc->mtx);
    unsigned char *c = (unsigned char *)cc;
    for (; n; c++, n--) {
        if (*c <= 032)
            vcons_put(vc, *c);  //te znaki mog± byæ nawet w ¶rodku escape
        else if (vc->escape) {
            int code = vc_parser_put(&vc->parser, *c);
            if (code == PARSER_ERROR) {
                vc->escape = FALSE;
            } else
            if (code != PARSER_CONT) {
                vcons_code(vc, code);
                vc->escape = FALSE;
            }
        } else {
            if (*c == CODE_ESC) {
                vc->escape = TRUE;
                vc_parser_reset(&vc->parser);
            } else if(*c == 0x9b) {
                vc->escape = TRUE;
                vc_parser_resetE(&vc->parser);
            } else {
                vcons_put(vc, *c);
            }
        }
    }
    mutex_unlock(&vc->mtx);
    splx(X);
}


void
vcons_put(vconsole_t *vc, char c)
{
    int cx, cy;
    textscreen_get_cursor(&vc->screen, &cx, &cy);
    
    if (isprint(c)) {
        if(cx == TS_WIDTH-1 && ISUNSET(vc->mode, CONS_MODE_AWRAP))
            textscreen_putat(&vc->screen, cx, cy, c, vc->sattr);
        else
            textscreen_put(&vc->screen, c, vc->sattr);
    } else if(c == NUL); //nie jestem pewien, czy dostawaæ tu taki znak
    else if (c == NL || c == VT || c == FF) {
        if(ISSET(vc->mode, CONS_MODE_NEWLINE))
            textscreen_next_line(&vc->screen);
        else
            textscreen_move_down(&vc->screen);
    } else if (c == CR) {
        textscreen_update_cursor(&vc->screen, 0, cy);
    } else if (c == HT) {
        textscreen_tab(&vc->screen);
    } else if (c == BS) {
        textscreen_move_cursor(&vc->screen, -1, 0);
    } else if (c == ENQ) {
        //todo
    } else if (c == SO || c == SI) {
        //sth...
    } else {
        char hex[16]="0123456789abcdef";
        textscreen_put(&vc->screen, '0', vc->sattr);
        textscreen_put(&vc->screen, 'x', vc->sattr);
        textscreen_put(&vc->screen, hex[((c&0xf0)>>4)], vc->sattr);
        textscreen_put(&vc->screen, hex[(c&0x0f)], vc->sattr);
    }
}

void
set_mode(vconsole_t *vc, int m)
{
    switch(m) {
        case 3: //column mode
            textscreen_clear(&vc->screen);
            SET(vc->mode, CONS_MODE_NEWLINE); //mo¿e?
            break;
        case 6: //origin avs
            SET(vc->mode, CONS_MODE_ORIGIN);
            textscreen_set_index_mode(&vc->screen, VIDEO_INDEX_ABSOLUTE);
            break;
        case 7: //autowrap
            SET(vc->mode, CONS_MODE_AWRAP);
            break;
        case 20: //line
            SET(vc->mode, CONS_MODE_NEWLINE);
            break;
        default:
            break;
    }
}

void
reset_mode(vconsole_t *vc, int m)
{
    switch(m) {
        case 3:
            textscreen_clear(&vc->screen);
            SET(vc->mode, CONS_MODE_NEWLINE); //mo¿e?
            break;
        case 6: //origin rel
            UNSET(vc->mode, CONS_MODE_ORIGIN);
            textscreen_set_index_mode(&vc->screen, VIDEO_INDEX_RELATIVE);
            break;
        case 7: //no autowrap
            UNSET(vc->mode, CONS_MODE_AWRAP);
            break;
        case 20: //line only vertical
            UNSET(vc->mode, CONS_MODE_NEWLINE);
            break;
        default:
            break;
    }
}

void
vcons_code(vconsole_t *vc, int c)
{
    int cx,cy;
    int attr0 =  vc->parser.attr[0];
    textscreen_get_cursor(&vc->screen, &cx, &cy);
    switch (c) {
        case ESC_SCROLL: {
            int attr1 = vc->parser.attr[1];
            if(attr0 == 0 && attr1 == 0)
                ;//...
            else {
                textscreen_set_margins(&vc->screen, attr0, attr1);
                textscreen_update_cursor(&vc->screen, 0, 0);
            }
            break;
        }
        case ESC_CURHOME:
            cx = vc->parser.attr[1]-1;
            cy = vc->parser.attr[0]-1;
            textscreen_update_cursor(&vc->screen, cx, cy);
            break;
        case ESC_CURSAVEA:
            vc->sattr2 = vc->sattr;
        case ESC_CURSAVE:
            vc->cx = cx;
            vc->cy = cy;
            break;
        case ESC_CURLOADA:
            vc->sattr = vc->sattr2;
        case ESC_CURLOAD:
            textscreen_update_cursor(&vc->screen, vc->cx, vc->cy);
            break;
        case ESC_TABCLR:
            if(attr0 == 0)
                textscreen_del_tab(&vc->screen);
            else if(attr0 == 3)
                textscreen_del_all_tab(&vc->screen);
            break;
        case ESC_TABSET:
            textscreen_set_tab(&vc->screen);
            break;
        case ESC_NEXTL:
            textscreen_next_line(&vc->screen);
            break;
        case ESC_MOVEU:
            textscreen_move_up(&vc->screen);
            break;
        case ESC_MOVED:
            textscreen_move_down(&vc->screen);
            break;
        case ESC_SETMODE:
            for(int i=0; i<=vc->parser.attr_i; i++)
                set_mode(vc, vc->parser.attr[i]);
            break;
        case ESC_RESMODE:
            for(int i=0; i<=vc->parser.attr_i; i++)
                reset_mode(vc, vc->parser.attr[i]);
            break;
        case ESC_CURUP:
            if(attr0==0)
                attr0++;
            textscreen_move_cursor(&vc->screen, 0, -attr0);
            break;
        case ESC_CURDOWN:
            if(attr0==0)
                attr0++;
            textscreen_move_cursor(&vc->screen, 0, attr0);
            break;
        case ESC_CURFORW:
            if(attr0==0)
                attr0++;
            textscreen_move_cursor(&vc->screen, attr0, 0);
            break;
        case ESC_CURBACK:
            if(attr0==0)
                attr0++;
            textscreen_move_cursor(&vc->screen, -attr0, 0);
            break;
        case ESC_RESET:
            textscreen_clear(&vc->screen);
            vc->sattr = TS_FG(DEFAULT_FG)|TS_BG(DEFAULT_BG);
            break;
        case ESC_DECALN:
            textscreen_fill(&vc->screen, 'E');
            break;
        case ESC_ERASEB:
            textscreen_clear_left(&vc->screen);
            break;
        case ESC_ERASEE:
            textscreen_clear_right(&vc->screen);
            break;
        case ESC_ERASEL:
            textscreen_clear_line(&vc->screen, cy);
            break;
        case ESC_ERASES:
            textscreen_clear(&vc->screen);
            break;
        case ESC_ERASEU:
            textscreen_clear_up(&vc->screen);
            break;
        case ESC_ERASED:
            textscreen_clear_down(&vc->screen);
            break;
        case ESC_ATTR:
            for (int i = 0; i <= vc->parser.attr_i; i++) {
                int a = vc->parser.attr[i];
                if (a < 10) {
                    switch (a) {
                        case 0:
                            vc->sattr = TS_FG(DEFAULT_FG)|TS_BG(DEFAULT_BG);
                            break;
                        case 1:
                            vc->sattr = TS_BOLD(vc->sattr);
                            break;
                        case 5: // mruganie
                            break;
                        case 7:
                            vc->sattr = TS_BG(_TS_FG(vc->sattr)) |
                                TS_FG(_TS_BG(vc->sattr));
                            break;
                    }
                } else
                if (30 <= a && a < 40) {
                    vc->sattr = _TS_BG(vc->sattr) | _TS_BOLD(vc->sattr)
                        | tty_fgattr_map[a-30];
                } else
                if (40 <= a && a < 50) {
                    vc->sattr = _TS_FG(vc->sattr) | _TS_BOLD(vc->sattr)
                        | tty_bgattr_map[a-40];
                }
            }
            break;
    }
}



/*========================================================================
 * Implementacja parsera skanuj±cego sekwencje steruj±ce VT100
 */


enum {
    P_DUMMY,
    P_FIRST,         // czekamy na pierwszy znak
    P_AFTER_HASH,    // jeste¶my po #    
    P_LONG,          // czekamy na znaczki po '['
    P_LONG_ATTR,     // czekamy na atrybut po '['
    P_SET_G0,        // znaczki po '('
    P_SET_G1         // znaczki po ')'

};

void
vc_parser_reset(vc_parser_t *vcprs)
{
    mem_zero(vcprs, sizeof(vc_parser_t));
    vcprs->state = P_FIRST;
}

void
vc_parser_resetE(vc_parser_t *vcprs)
{
    mem_zero(vcprs, sizeof(vc_parser_t));
    vcprs->state = P_LONG;
}

/*
 * Zastanawiam siê czy nie przepisaæ tego na jaki¶ DFA.
 */
int
vc_parser_put(vc_parser_t *vcprs, char c)
{
    int ret = PARSER_CONT;
    int nexts = P_DUMMY;
    if(vcprs->state == P_FIRST) {
        switch (c) {
            case 'c':
                ret = ESC_RESET;
                break;
            case '(':
                nexts = P_SET_G0;
                break;
            case ')':
                nexts = P_SET_G1;
                break;
            case '7':
                ret = ESC_CURSAVEA;
                break;
            case '8':
                ret = ESC_CURLOADA;
                break;
            case 'D':
                ret = ESC_MOVED;
                break;
            case 'E':
                ret = ESC_NEXTL;
                break;    
            case 'M':
                ret = ESC_MOVEU;
                break;
            case 'H':
                ret = ESC_TABSET;
                break;
            case '[':
                nexts = P_LONG;
                break;
            case '#':
                nexts = P_AFTER_HASH;
                break;
            default:
                ret = PARSER_ERROR;
                break;
        }
    } else if (vcprs->state == P_AFTER_HASH) {
        switch (c) {
            case '8':
                ret = ESC_DECALN;
                break;
            default:
                ret = PARSER_ERROR;
                break;
        }
    } else if(vcprs->state == P_SET_G0) {
        switch (c) {
            default:
                ret = PARSER_ERROR;
                break;
        }
    } else if(vcprs->state == P_SET_G1) {
                switch (c) {
            default:
                ret = PARSER_ERROR;
                break;
        }
    } else if (vcprs->state == P_LONG) {
        if (c == '?') {
            nexts = P_LONG_ATTR;
        } else if ( '0' <= c && c <= '9') {
            vcprs->attr[0] = c - '0';
            vcprs->attr_i = 0;
            nexts = P_LONG_ATTR;
        } else
        switch (c) {
            case ';':
                nexts = P_LONG_ATTR;
                vcprs->attr[0] = 0;
                vcprs->attr_i++;
                vcprs->attr[vcprs->attr_i] = 0;
                break;
            case 's':
                ret = ESC_CURSAVE;
                break;
            case 'u':
                ret = ESC_CURLOAD;
                break;
            case 'r':
                ret = ESC_SCROLL;
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
            default:
                ret = PARSER_ERROR;
                break;
        }
    } else if (vcprs->state == P_LONG_ATTR) {
        if ('0' <= c && c <= '9') {
            vcprs->attr[vcprs->attr_i] *= 10;
            vcprs->attr[vcprs->attr_i] += c - '0';
            nexts = P_LONG_ATTR;
        } else
        switch (c) {
            case ';':
                nexts = P_LONG_ATTR;
                vcprs->attr_i++;
                vcprs->attr[vcprs->attr_i] = 0;
                break;
            case 'm':
                ret = ESC_ATTR;
                break;
            case 'h':
                ret = ESC_SETMODE;
                break;
            case 'l':
                ret = ESC_RESMODE;
                break;
            case 'f':
            case 'H':
                ret = ESC_CURHOME;
                break;
            case 'g':
                ret = ESC_TABCLR;
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
            case 'K':
                switch (vcprs->attr[0]) {
                    case 0:
                        ret = ESC_ERASEE;
                        break;
                    case 1:
                        ret = ESC_ERASEB;
                        break;
                    case 2:
                        ret = ESC_ERASEL;
                        break;
                    default:
                        ret = PARSER_ERROR;
                        break;
                }
                break;
            case 'J':
                switch (vcprs->attr[0]) {
                    case 0:
                        ret = ESC_ERASED;
                        break;
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
                switch (vcprs->attr[0]) {
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
    if (vcprs->attr_i == PARSER_MAX_ATTRS) {
        vcprs->attr_i = 0;
        ret = PARSER_ERROR;
    }
    vcprs->state = nexts;
    return ret;
}
