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
#include <sys/utils.h>
#include <sys/string.h>
#include <sys/vm.h>
#include <machine/video.h>
#include <machine/io.h>
static int8_t forced_attr = 0;

static struct hw_textscreen  defscreen;
static struct hw_textscreen *current = NULL;

#define SELECT_SCREEN(ptr) (((ptr)==NULL)? current : ptr)
#define SELECT_MAP(ptr) ((ptr->screen_buf)? ptr->screen_buf : ptr->screen_map)
#define TS_VIDEO   (vm_paddr_t)0xb8000

uint16_t *vidmem = NULL;

enum {
    VGA_PORT = 0x3d4
};

enum {
    REG_MAX_SCANLINE = 0x9,
    REG_CUR_START = 0x0a,
    REG_CUR_END = 0x0b,
    REG_CUR_POS_HI = 0x0e,
    REG_CUR_POS_LO = 0x0f
};

void
video_init()
{
#if 0
    io_out8(VGA_PORT, REG_MAX_SCANLINE);
    uint8_t line = io_in8(VGA_PORT+1);
#endif
    io_out8(VGA_PORT, REG_CUR_START);
    io_out8(VGA_PORT+1, 1 << 5);
#if 0
    io_out8(VGA_PORT, REG_CUR_END);
    io_out8(VGA_PORT+1, 0x1f & line);
#endif
    io_out8(VGA_PORT, REG_CUR_POS_HI);
    uint16_t cur_pos = io_in8(VGA_PORT+1) << 8;
    io_out8(VGA_PORT, REG_CUR_POS_LO);
    cur_pos |=  io_in8(VGA_PORT+1);
    vidmem = (void*)TS_VIDEO;


    vm_physmap(TS_VIDEO, TS_SIZE*2, &vidmem);

    defscreen.cursor_x = cur_pos % TS_WIDTH;
    defscreen.cursor_y = cur_pos / TS_WIDTH;
    defscreen.screen_buf = (uint16_t*) vidmem;
    defscreen.cursor_hack = 0;
    current = &defscreen;
    defscreen.cursor_hack = defscreen.screen_buf[cur_pos];
    SYSTEM_DEBUG = 1;


}

void
textscreen_init(struct hw_textscreen *ts)
{
    ts->screen_buf = NULL;
    textscreen_clear(ts);
}


void
textscreen_enable_forced_attr(int8_t f)
{
    forced_attr = f;
}

void
textscreen_disable_forced_attr()
{
    forced_attr = 0;
}

void
textscreen_next_line(struct hw_textscreen *screen)
{
    screen = SELECT_SCREEN(screen);
    if (screen->screen_buf) {
        uint16_t *map = screen->screen_buf;
        int cur_pos = screen->cursor_y*TS_WIDTH + screen->cursor_x;
        map[cur_pos] = screen->cursor_hack;
    }

    if ( screen->cursor_y < TS_HEIGHT-1 )
        textscreen_update_cursor(screen, 0, screen->cursor_y+1);
    else
        textscreen_scroll(screen);

}


void
textscreen_putat(struct hw_textscreen *screen, int8_t col, int8_t row,
        char c, int8_t attribute)
{
    screen = SELECT_SCREEN(screen);
    uint16_t *map = SELECT_MAP(screen);
    int cur_pos = (screen->cursor_y) * TS_WIDTH +
        screen->cursor_x;
    int pos = TS_WIDTH*row+col;
    if (pos == cur_pos)
        screen->cursor_hack = attribute<<8 | c;
    map[TS_WIDTH*row+col] = (uint16_t)attribute<<8|c;
}

void
textscreen_put(struct hw_textscreen *screen, char c, int8_t attr)
{
    if (forced_attr) attr = forced_attr;
    screen = SELECT_SCREEN(screen);

    textscreen_putat(screen, screen->cursor_x, screen->cursor_y, c, attr);

    if ( screen->cursor_x == TS_WIDTH-1 )
    {
        if ( screen->cursor_y < TS_HEIGHT-1 )
            textscreen_update_cursor(screen, 0, screen->cursor_y+1);
        else
            textscreen_scroll(screen);
    }
    else
        textscreen_update_cursor(screen, screen->cursor_x+1,
                screen->cursor_y);
}

void
textscreen_update_cursor(struct hw_textscreen *screen, int8_t col,
        int8_t row)
{
    screen = SELECT_SCREEN(screen);

    if (col < 0) col = 0;
    if (row < 0) row = 0;
    if (col > TS_WIDTH-1) col = TS_WIDTH-1;
    if (row > TS_HEIGHT-1) row = TS_HEIGHT-1;
    // zapamietujemy poprzednia pozycje
    int cur_pos = (screen->cursor_y) * TS_WIDTH +
        screen->cursor_x;

    screen->cursor_y = row;
    screen->cursor_x = col;

    if (screen->screen_buf) {
        screen->screen_buf[cur_pos] = screen->cursor_hack;
        cur_pos = (screen->cursor_y) * TS_WIDTH + screen->cursor_x;
#if 0
        io_out8(VGA_PORT, REG_CUR_POS_HI);
        io_out8(VGA_PORT+1, cur_pos>>8 );
        io_out8(VGA_PORT, REG_CUR_POS_LO);
        io_out8(VGA_PORT+1, cur_pos & 0x00ff);
#endif
        screen->cursor_hack = screen->screen_buf[cur_pos];
        screen->screen_buf[cur_pos] = (screen->cursor_hack) |
            (TS_FG(COLOR_WHITE) | TS_BG(COLOR_BRIGHTGRAY)) << 8;
    }
}

void
textscreen_get_cursor(struct hw_textscreen *screen, int *cx, int *cy)
{
    screen = SELECT_SCREEN(screen);
    *cx = screen->cursor_x;
    *cy = screen->cursor_y;
}

void
textscreen_scroll(struct hw_textscreen *screen)
{
    screen = SELECT_SCREEN(screen);
    uint16_t *map = SELECT_MAP(screen);

    mem_move(map, &map[TS_WIDTH],
                24*TS_WIDTH*sizeof(uint16_t));

    mem_set16(&map[24*TS_WIDTH], COLOR_WHITE<<8 | ' ',
            TS_WIDTH*sizeof(uint16_t));
    
    screen->cursor_y--;
    textscreen_update_cursor(screen, 0, screen->cursor_y+1);
}

void
textscreen_clear(struct hw_textscreen *screen)
{
    screen = SELECT_SCREEN(screen);
    uint16_t *map = SELECT_MAP(screen);
    for ( int i = 0 ; i < TS_SIZE ; i++ )
        map[i] = COLOR_WHITE<<8;
    textscreen_update_cursor(screen, 0, 0);
}

void
textscreen_reset(struct hw_textscreen *screen)
{
    screen = SELECT_SCREEN(screen);
    textscreen_clear(screen);
}

void
textscreen_draw(struct hw_textscreen *screen)
{
    current = screen;
    mem_cpy(vidmem, screen->screen_map,
            TS_SIZE*sizeof(uint16_t));
    screen->screen_buf = (uint16_t*) vidmem;
    textscreen_update_cursor(screen, screen->cursor_x, screen->cursor_y);
}

void
textscreen_switch(struct hw_textscreen *screen)
{
    current->screen_buf = 0;
    textscreen_draw(screen);
}

void
textscreen_clone(struct hw_textscreen *screen)
{
    mem_cpy(screen, current, sizeof(*screen));
    mem_cpy(screen->screen_map, current->screen_buf, TS_SIZE*2);
    screen->screen_buf = NULL;
}

