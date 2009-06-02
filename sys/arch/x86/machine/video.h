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

#ifndef __MACHINE_VIDEO_H
#define __MACHINE_VIDEO_H

#include <sys/types.h>

enum {
    TS_WIDTH = 80,
    TS_HEIGHT = 25,
    TS_SIZE =  (TS_WIDTH*TS_HEIGHT)
};

#define _TS_BG(attr) (((attr) >> 4) & 0x7)
#define _TS_FG(attr) ((attr) & 0xf)
#define _TS_BOLD(attr) (attr & 0x8)

#define TS_BG(attr) (((attr) & 0xf) << 4)
#define TS_FG(attr) ((attr) & 0x7)
#define TS_BOLD(attr) (attr | 0x8)

typedef struct hw_textscreen textscreen_t;
struct hw_textscreen {
    uint16_t screen_map[TS_SIZE];
    uint16_t *screen_buf;
    int8_t cursor_y;
    int8_t cursor_x;
    uint16_t cursor_hack;
};

void textscreen_enable_forced_attr(int8_t f);
void textscreen_disable_forced_attr(void);

void video_init(void);
void textscreen_init(struct hw_textscreen *ts);

void textscreen_putat(struct hw_textscreen *screen, int8_t col, int8_t row,
        char c, int8_t attribute);
void textscreen_put(struct hw_textscreen *screen, char c,
        int8_t attr);
void textscreen_scroll(struct hw_textscreen *screen);
void textscreen_update_cursor(struct hw_textscreen *screen, int8_t col,
        int8_t row);
void textscreen_next_line(struct hw_textscreen *screen);
void textscreen_reset(struct hw_textscreen *screen);
void textscreen_clear(struct hw_textscreen *screen);
void textscreen_draw(struct hw_textscreen *screen);
void textscreen_switch(struct hw_textscreen *screen);
void textscreen_clone(struct hw_textscreen *screen);
void textscreen_get_cursor(struct hw_textscreen *screen, int *cx, int *cy);

enum {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_BROWN,
    COLOR_BRIGHTGRAY,
    COLOR_DARKGRAY,
    COLOR_BRIGHTBLUE,
    COLOR_BRIGHTGREEN,
    COLOR_BRIGHTCYAN,
    COLOR_BRIGHTRED,
    COLOR_BRIGHTMAGENTA,
    COLOR_BRIGHTBROWN,
    COLOR_WHITE
};

#endif
