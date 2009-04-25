/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/libkutil.h>
#include <machine/video.h>
#include <machine/io.h>

static int8_t forced_attr = 0;

struct hw_textscreen textscreen;

void
textscreen_init()
{
//    mem_zero(&textscreen, sizeof(textscreen));

    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x9);
    uint8_t line = io_in8(TEXTSCREEN_VIDPORT_DATA);
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0xa);
    io_out8(TEXTSCREEN_VIDPORT_DATA, 0);
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0xb);
    io_out8(TEXTSCREEN_VIDPORT_DATA, line-2);

    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0f);
    uint8_t cur_pos = io_in8(TEXTSCREEN_VIDPORT_DATA);
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0e);
    cur_pos |= io_in8(TEXTSCREEN_VIDPORT_DATA) << 8;

    textscreen.cursor_x = cur_pos % TEXTSCREEN_WIDTH;
    textscreen.cursor_y = cur_pos / TEXTSCREEN_WIDTH;
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
textscreen_putat(struct hw_textscreen *screen, int8_t col, int8_t row, 
        char c, int8_t attribute)
{
/*
    if (TEXTSCREEN_WIDTH*row+col > TEXTSCREEN_WIDTH*TEXTSCREEN_HEIGHT) {
        char *x = (char*) 0xb8000;
        x[0] = 'A';
        x[1] = '0';
        for (;;);
    }
*/
    screen->screen_map[TEXTSCREEN_WIDTH*row+col] = (uint16_t)attribute<<8|c;
}

void
textscreen_put(struct hw_textscreen *screen, char c, int8_t attr)
{
    if (forced_attr) attr = forced_attr;

    textscreen_putat(screen, screen->cursor_x, screen->cursor_y, c, attr);

    /* uaktualniamy pozycje kursora */

    /* sprawdzamy czy jestesmy na koncu wiersza */
    if ( screen->cursor_x == TEXTSCREEN_WIDTH-1 )
    {
        /* sprawdzamy czy mozna przenisc kursor do nastepnej linii */
        if ( screen->cursor_y < TEXTSCREEN_HEIGHT-1 )
            /* przenosimy kursor do nastepnej linii */
            textscreen_update_cursor(screen, 0, screen->cursor_y+1);
        else
            /* scrollujemy ekran i przenosimy kursor na poczatek linii */
            textscreen_scroll(screen);
    }
    else
        /* kursor nie jest na koncu wiersza wiec uaktualniamy o jedna pozycje
         * w prawo */
        textscreen_update_cursor(screen, screen->cursor_x+1, 
                screen->cursor_y);
}

void
textscreen_update_cursor(struct hw_textscreen *screen, int8_t col,
        int8_t row)
{
    screen->cursor_y = row;
    screen->cursor_x = col;
}

void
textscreen_scroll(struct hw_textscreen *screen)
{
    /* przesuwamy ekran o linie w gore */
    mem_move(screen->screen_map, &screen->screen_map[TEXTSCREEN_WIDTH],
                24*TEXTSCREEN_WIDTH*sizeof(uint16_t));
    
    /* kasujemy ostatnia linie */
    mem_zero(&screen->screen_map[24*TEXTSCREEN_WIDTH], 
            TEXTSCREEN_WIDTH*sizeof(uint16_t));
    
    /* uaktualniamy kursor */
    textscreen_update_cursor(screen, 0, screen->cursor_y);
}

void
textscreen_clear(struct hw_textscreen *screen)
{
    uint16_t i; /* zmienna pomocnicza */

    /* ustawiamy domyslne atrybuty */
    for ( i = 0 ; i < TEXTSCREEN_WIDTH*TEXTSCREEN_HEIGHT ; i++ )
        screen->screen_map[i] = COLOR_WHITE<<8;

    /* ustawiamy kursor na poczatek */
    textscreen_update_cursor(screen, 0, 0);
}

void
textscreen_reset(struct hw_textscreen *screen)
{
    textscreen_clear(screen);
}

void
textscreen_draw(struct hw_textscreen *screen)
{
    uint16_t cur_pos = (screen->cursor_y) * TEXTSCREEN_WIDTH + 
        screen->cursor_x; /* cursor position */

    /* kopiowanie screen_map na ekran */

    screen->screen_map[cur_pos] = (COLOR_WHITE<<8)|219;

    mem_cpy(TEXTSCREEN_VIDEO, screen->screen_map,
            TEXTSCREEN_WIDTH*TEXTSCREEN_HEIGHT*sizeof(uint16_t));

    /* uaktualnienie pozycji kursora */

    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0f); 
    io_out8(TEXTSCREEN_VIDPORT_DATA, (uint8_t)cur_pos);
    io_out8(TEXTSCREEN_VIDPORT_IDX, 0x0e); 
    io_out8(TEXTSCREEN_VIDPORT_DATA, cur_pos>>8);

}
