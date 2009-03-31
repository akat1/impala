/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_PCKBD_TABLES_H
#define __MACHINE_PCKBD_TABLES_H


/// kody klawiszy
enum {
    KC_CAPSLOCK = 0x3a,
    KC_DELETE = 127,
    KC_LCTRL = 129,
    KC_RCTRL,
    KC_LALT,
    KC_RALT,
    KC_INSERT,
    KC_HOME,
    KC_END,
    KC_PGUP,
    KC_PGDOWN,
    KC_LEFT,
    KC_UP,
    KC_DOWN,
    KC_RIGHT,
    KC_KPAD_DIV,
    KC_KPAD_ENTER,
    KC_PRTSCR,
    KC_CTRL_BREAK,
    KC_LWIN,
    KC_RWIN,
    KC_MENU,
    KC_SLEEP,
    KC_POWER,
    KC_WAKE,
    KC_PAUSE
};


/// tablica do translacji scancode -> keycode dla klawiszy z grupy 'e0'
uint8_t e0_kcodes[] = 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x00 - 0x0f
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KC_RCTRL, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KC_PRTSCR, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, KC_PRTSCR, KC_RALT, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, KC_HOME,
KC_UP, KC_PGUP, 0, KC_LEFT, 0, KC_RIGHT, 0, KC_END,
KC_DOWN, KC_PGDOWN, KC_INSERT, KC_DELETE, 0, 0, 0, 0,
0, 0, 0, KC_LWIN, KC_RWIN, KC_MENU, KC_POWER, KC_SLEEP,
0, 0, 0, KC_WAKE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/// tablica do translacji keycode -> ascii w przypadku braku wci¶niêtych
/// klawiszy modyfikuj±cych
uint8_t keymap_normal[] =
{ 0,'\e', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=','\b', '\t',// 0x00 - 0x0f
'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  0, 'a', 's', // 0x10 - 0x1f  //0x1d - brak
'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`', 0 , '\\', 'z', 'x', 'c', 'v', // 0x20 - 0x2f  //0x2a - lshift
'b', 'n', 'm', ',', '.', '/', 0  , '*', 0  , ' ', 0 ,  0 ,    0,   0,   0,   0, // 0x30 - 0x3f  //0x36 - rshift
  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40 - 0x4f
'2', '3', '0', '.',   0,   0,   0,   0                                          // 0x50 - 0x58
};

uint8_t keymap_shift[] =
{ 0,'\e', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+','\b', '\t',// 0x00 - 0x0f
'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',  0, 'A', 'S', // 0x10 - 0x1f  //0x1d - brak
'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0 , '|', 'Z', 'X', 'C', 'V', // 0x20 - 0x2f  //0x2a - lshift
'B', 'N', 'M', '<', '>', '?', 0  , '*', 0  , ' ', 0 ,  0 ,    0,   0,   0,   0, // 0x30 - 0x3f  //0x36 - rshift
  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40 - 0x4f
'2', '3', '0', '.',   0,   0,   0,   0                                          // 0x50 - 0x58
};

/// flagi dla klawiszy modyfikuj±cych zachowanie pozosta³ych klawiszy
enum {
    KM_LSHIFT   = 1<<0,
    KM_RSHIFT   = 1<<1,
    KM_LCONTROL = 1<<2,
    KM_RCONTROL = 1<<3,
    KM_ALT      = 1<<4,
    KM_ALTGR    = 1<<5,
    KM_CAPSLOCK = 1<<6,
    KM_MENU     = 1<<7
};

/// tablica z flagami oraz keycode'ami klawiszy modyfikuj±cych
uint8_t key_modifiers_tab[8][2] = {
{KM_LSHIFT, 0x2a},
{KM_RSHIFT, 0x36},
{KM_LCONTROL, 0x1d},
{KM_RCONTROL, KC_RCTRL},
{KM_ALT, KC_LALT},
{KM_ALTGR, KC_RALT},
{KM_CAPSLOCK, KC_CAPSLOCK},
{KM_MENU, KC_MENU}};
 

  
#endif
