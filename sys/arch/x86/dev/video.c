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
#include <sys/libkutil.h>
#include <machine/video.h>
#include <machine/io.h>
#include <sys/kprintf.h>
static int8_t forced_attr = 0;

static struct hw_textscreen  defscreen;
static struct hw_textscreen *current;

#define SELECT_SCREEN(ptr) (((ptr)==NULL)? current : ptr)
#define SELECT_MAP(ptr) ((ptr->screen_buf)? ptr->screen_buf : ptr->screen_map)

void
textscreen_init()
{   
//    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x9);
    uint8_t line = io_in8(TEXTSCREEN_VIDPORT_DATA);
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0xa);
    io_out8(TEXTSCREEN_VIDPORT_DATA, 0);
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0xb);
    io_out8(TEXTSCREEN_VIDPORT_DATA, line-2);

    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0e);
    uint16_t cur_pos = io_in8(TEXTSCREEN_VIDPORT_DATA) << 8;
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0f);
    cur_pos |=  io_in8(TEXTSCREEN_VIDPORT_DATA);
    defscreen.cursor_x = cur_pos % TEXTSCREEN_WIDTH;
    defscreen.cursor_y = cur_pos / TEXTSCREEN_WIDTH;
    defscreen.screen_buf = (uint16_t*) TEXTSCREEN_VIDEO;
//    mem_set16(defscreen.screen_buf, COLOR_GREEN<<8 | ' ', TEXTSCREEN_WIDTH*TEXTSCREEN_HEIGHT*2);
    current = &defscreen;
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
    if ( screen->cursor_y < TEXTSCREEN_HEIGHT-1 )
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
    map[TEXTSCREEN_WIDTH*row+col] = (uint16_t)attribute<<8|c;
}

void
textscreen_put(struct hw_textscreen *screen, char c, int8_t attr)
{
    if (forced_attr) attr = forced_attr;
    screen = SELECT_SCREEN(screen);

    textscreen_putat(screen, screen->cursor_x, screen->cursor_y, c, attr);

    if ( screen->cursor_x == TEXTSCREEN_WIDTH-1 )
    {
        if ( screen->cursor_y < TEXTSCREEN_HEIGHT-1 )
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

    screen->cursor_y = row;
    screen->cursor_x = col;

    if (screen->screen_buf) {
        uint16_t cur_pos = (screen->cursor_y) * TEXTSCREEN_WIDTH +
            screen->cursor_x;

        io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0f);
        io_out8(TEXTSCREEN_VIDPORT_DATA, cur_pos & 0xff);
        io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0e);
        io_out8(TEXTSCREEN_VIDPORT_DATA, (cur_pos>>8) & 0xff);
    }
}

void
textscreen_scroll(struct hw_textscreen *screen)
{
    screen = SELECT_SCREEN(screen);
    uint16_t *map = SELECT_MAP(screen);

    mem_move(map, &map[TEXTSCREEN_WIDTH],
                24*TEXTSCREEN_WIDTH*sizeof(uint16_t));
    
    mem_set16(&map[24*TEXTSCREEN_WIDTH], COLOR_WHITE<<8 | ' ',
            TEXTSCREEN_WIDTH*sizeof(uint16_t));
    
    textscreen_update_cursor(screen, 0, screen->cursor_y);

}

void
textscreen_clear(struct hw_textscreen *screen)
{
    screen = SELECT_SCREEN(screen);
    uint16_t *map = SELECT_MAP(screen);
    uint16_t i;

    for ( i = 0 ; i < TEXTSCREEN_WIDTH*TEXTSCREEN_HEIGHT ; i++ )
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
    if (screen->screen_buf) return;
    uint16_t cur_pos = (screen->cursor_y) * TEXTSCREEN_WIDTH + 
        screen->cursor_x; 

//     screen->screen_map[cur_pos] = (COLOR_WHITE<<8)|219;

    mem_cpy(TEXTSCREEN_VIDEO, screen->screen_map,
            TEXTSCREEN_WIDTH*TEXTSCREEN_HEIGHT*sizeof(uint16_t));

    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0f); 
    io_out8(TEXTSCREEN_VIDPORT_DATA, cur_pos & 0xff);
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0e); 
    io_out8(TEXTSCREEN_VIDPORT_DATA, (cur_pos>>8) & 0xff);
    screen->screen_buf = (uint16_t*) TEXTSCREEN_VIDEO;
}
