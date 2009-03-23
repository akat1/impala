#ifndef __MACHINE_VIDEO_H
#define __MACHINE_VIDEO_H

#include <sys/types.h>

#define TEXTSCREEN_VIDEO   (addr_t)0xb8000
#define TEXTSCREEN_VIDPORT_IDX  0x3d4
#define TEXTSCREEN_VIDPORT_DATA 0x3d5
#define TEXTSCREEN_WIDTH   80
#define TEXTSCREEN_HEIGHT  25
#define TEXTSCREEN_SIZE (TEXTSCREEN_WIDTH*TEXTSCREEN_HEIGHT)

struct hw_textscreen {
    uint16_t screen_map[TEXTSCREEN_SIZE];
    int8_t cursor_y;
    int8_t cursor_x;
};

/* tymczasowo na ekran */
struct hw_textscreen textscreen;

void textscreen_enable_forced_attr(int8_t f);
void textscreen_disable_forced_attr(void);


void textscreen_putat(struct hw_textscreen *screen, int8_t col, int8_t row,
        char c, int8_t attribute);
void textscreen_put(struct hw_textscreen *screen, char c, 
        int8_t attr);
void textscreen_scroll(struct hw_textscreen *screen);
void textscreen_update_cursor(struct hw_textscreen *screen, int8_t col, 
        int8_t row);
void textscreen_reset(struct hw_textscreen *screen);
void textscreen_clear(struct hw_textscreen *screen);
void textscreen_draw(struct hw_textscreen *screen);

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
