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
#include <machine/video.h>

typedef struct vtty vtty_t;
struct vtty {
    int num;
    int bold;
    int sattr;
    struct hw_textscreen screen;
};

enum {
    VTTY_MAX
};

static void vtty_put(vtty_t *t, char c);
static void vtty_out(vtty_t *t, const char *c);
static vtty_t *vttys[VTTY_MAX];
static vtty_t *current_vtty = NULL;

static int tty2video_fgattr_map[10] = {
    TS_FG(COLOR_BLACK),
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

static int tty2video_bgattr_map[10] = {
    TS_BG(COLOR_BLACK),
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

void
vtty_out(vtty_t *vt, const char *c)
{
    enum {
        CODE_ESC = 033
        NOESC,
        NO
    };
    char buf[4];
    int bufi;
    int state_ESC = NOESC;
    for (; *c != 0; c++) {
        if (state_ESC) {
          
        } else {
            if (*c == CODE_ESC) {
                bufi = 0;
            } else {
                vtty_put(vt, *c);
            }
        }
    }
}

void
vtty_put(vtty_t *vt, char c)
{
    textscreen_put(&vt->screen, c, vt->sattr);
}

