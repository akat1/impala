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

#include <sys/types.h>
#include <sys/console.h>
#include <sys/kmem.h>
#include <sys/device.h>
#include <sys/string.h>
#include <machine/video.h>

enum {
    PARSER_MAX_BUF = 10
};

typedef struct vt_parser vt_parser_t;
struct vt_parser {
    int     state;
    char    buf[PARSER_MAX_BUF];
    int     bufidx;
    int     substate;
};


typedef struct vtty vtty_t;
struct vtty {
    int             num;
    int             bold;
    int             sattr;
    textscreen_t    screen;
    vt_parser_t     parser;
};

enum {
    VTTY_MAX
};

static void vtty_put(vtty_t *t, char c);
static void vtty_out(vtty_t *t, const char *c);
static vtty_t *vttys[VTTY_MAX];
static vtty_t *current_vtty = NULL;

// domy¶lne atrybuty
enum {
    DEFAULT_FG = COLOR_BRIGHTGRAY,
    DEFAULT_BG = COLOR_BLACK,
    HIDDEN = COLOR_DARKGRAY
};

#if 0
// Odwzorowanie kolorów czcionki z sekwencji steruj±cych na kolory VGA
static int tty2video_fgattr_map[10] = {
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
static int tty2video_bgattr_map[10] = {
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
#endif

/*========================================================================
 * Konsola
 */
void
cons_init()
{
    for (int i = 0; i < VTTY_MAX; i++) {
        vttys[i] = kmem_alloc(sizeof(vtty_t), KM_SLEEP);
        textscreen_clone(&vttys[i]->screen);
        if (i>0)
            textscreen_clear(&vttys[i]->screen);
    }
    textscreen_switch(&vttys[0]->screen);
    current_vtty = vttys[0];
}

void
cons_out(const char *c)
{
    vtty_out(current_vtty, c);
}

/*========================================================================
 * Obs³uga wirtualnych terminali w emulacji VT100
 */


static int vt_parser_put(vt_parser_t *vtprs, char c);
static void vt_parser_reset(vt_parser_t *vtprs);

/*
 * Sekwencji steruj±cy terminalu VT100.
 * Wszystkie zaczynaj± siê od znaku ESCAPE, klamrami {}
 * s± oznaczone zmienne. Zapis {X=y} oznacza, ¿e parametr
 * X mo¿e nie zostaæ podany, i wtedy przyjmuje domy¶ln± warto¶æ y.
 * Opis kodów dostêpny na stronie (kody poni¿ej s± w tej samej kolejno¶æi)
 *      http://www.termsys.demon.co.uk/vtansi.htm
 *
 * Dodatkowe oznaczenia:
 *  #   - kod jest rozpoznawany przez sterownik
 *  @   - kod jest rozpoznawany i obs³ugiwany przez sterownik
 * Przyk³adowe kody emulacji VT100:
 *      \033c           resetuje terminal
 *      \033[0m         resetuje atrybuty
 *      \033[31m        ustawia kolor czcionki na czerwony
 *      \033[32;47m     ustaiwa kolor czciocnki na zielony, a t³a na szary
 */
enum {
    ESC_RESET,      // c (#)
    ESC_LWRAPON,    // [7h 
    ESC_LWRAPOFF,   // [71 
    ESC_G0,         // ( (#)
    ESC_G1,         // ) (#)
    ESC_CURHOME,    // [{ROW=0};{COL=0}H 
    ESC_CURUP,      // [{COUNT=1}A 
    ESC_CURDOWN,    // [{COUNT=1}B  
    ESC_CURFORW,    // [{COUNT=1}C
    ESC_CURBACK,    // [{COUNT=1}D
    ESC_CURFORCE,   // [{ROW=-};{COL=0}f 
    ESC_CURSAVE,    // [s (#)
    ESC_CURLOAD,    // [u (#)
    ESC_CURSAVEA,   // 7 (#)
    ESC_CURLOADA,   // 8 (#)
    ESC_SCROLLE,    // [r (#)
    ESC_SCROLL,     // [{start};{end}r
    ESC_SCROLLD,    // D (#)
    ESC_SCROLLU,    // M (#)
    ESC_TABSET,     // H (#)
    ESC_TABCLR,     // [g (#)
    ESC_TABCLRA,    // [3g
    ESC_ERASEE,     // [K (#)
    ESC_ERASEB,     // [1K
    ESC_ERASEL,     // [2K
    ESC_ERASED,     // [J (#)
    ESC_ERASEU,     // [1J
    ESC_ERASES,     // [2J
    ESC_PRINTS,     // [i (#)
    ESC_PRINTL,     // [1i
    ESC_LOGS,       // [4i
    ESC_LOGE,       // [5i
    ESC_DEFKEY,     // [{key};"{string}"p
    ESC_ATTR,       // [{attr1};...;{attrn}m
};

// atrybuty ESC_ATTR
enum {
    ATTR_RESET      = 0,
    ATTR_BOLD       = 1,
    ATTR_DIM        = 2,
    ATTR_UNDER      = 4,
    ATTR_BLINK      = 5,
    ATTR_REVERSE    = 7,
    ATTR_HIDDEN     = 8
};

enum {
    PARSER_ERROR    = -1,
    PARSER_CONT     = -2,
};

/*========================================================================
 * Implementacja parsera skanuj±cego sekwencje steruj±ce VT100
 */

static void vtty_code(vtty_t *vt, int c);

void
vtty_out(vtty_t *vt, const char *c)
{
    enum {
        CODE_ESC = 033
    };
    bool escape = FALSE;

    for (; *c; c++) {
        if (escape) {
            int code = vt_parser_put(&vt->parser, *c); 
            if (code == PARSER_ERROR) {
                escape = FALSE;
            } else
            if (code != PARSER_CONT) {
                vtty_code(vt, code);
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
}

void
vtty_code(vtty_t *vt, int c)
{
    switch (c) {
        case ESC_RESET:
            break;
        case ESC_ATTR:
            break;
    }
}


void
vtty_put(vtty_t *vt, char c)
{
    textscreen_put(&vt->screen, c, vt->sattr);
}

enum {
    P_DUMMY,
    P_FIRST,         // czekamy na pierwszy znak
    P_INT,           // uzupe³niamy bufor na INT'a
    P_LONG           // czekamy na znaczki po '['

};


void
vt_parser_reset(vt_parser_t *vtprs)
{
    mem_zero(vtprs, sizeof(*vtprs));
    vtprs->state = P_FIRST;
    
}

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
            vtprs->buf[vtprs->bufidx] = c - '0';
            vtprs->bufidx++;
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
        }
    }
    if (vtprs->bufidx == PARSER_MAX_BUF) {
        vtprs->bufidx = 0;
        return PARSER_ERROR;
    }
    vtprs->state = nexts;
    return ret;
}

