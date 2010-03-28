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

#ifndef __MACHINE_PCKBD_TABLES_H
#define __MACHINE_PCKBD_TABLES_H

//ASCII control characters
#include <sys/ascii.h>

/// kody klawiszy
enum {
    KC_SPACE = 0x39,
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
    KC_PAUSE,
    KC_F1 = 0x3b,
    KC_F2,
    KC_F3,
    KC_F4,
    KC_F5,
    KC_F6,
    KC_F7,
    KC_F8,
    KC_F9,
    KC_F10,
    KC_F11 = 0x57,
    KC_F12,
};

char *keymap_string[] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "D",
     "A", "B", "C", NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
     NULL, NULL, NULL, NULL 
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

enum KEYCODE_TYPE {
    KT_NORMAL = 0,
    KT_SPECIAL = 1
};

uint8_t keycode_type[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x00 - 0x0f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x10 - 0x1f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x20 - 0x2f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, //0x30 - 0x3f
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x40 - 0x4f
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, //0x50 - 0x5f
};

/// tablica do translacji keycode -> ascii w przypadku braku wciśniętych
/// klawiszy modyfikujących
uint8_t keymap_normal[] =
{ 0,'\e', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=','\b', '\t',// 0x00 - 0x0f
'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\r',  0, 'a', 's', // 0x10 - 0x1f  //0x1d - brak
'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`', 0 , '\\', 'z', 'x', 'c', 'v', // 0x20 - 0x2f  //0x2a - lshift
'b', 'n', 'm', ',', '.', '/', 0  , '*', 0  , ' ', 0 ,  0 ,    0,   0,   0,   0, // 0x30 - 0x3f  //0x36 - rshift
  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40 - 0x4f
'2', '3', '0', '.',   0,   0,   0,   0                                          // 0x50 - 0x58
};

uint8_t keymap_shift[] =
{ 0,'\e', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+','\b', '\t',// 0x00 - 0x0f
'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\r',  0, 'A', 'S', // 0x10 - 0x1f  //0x1d - brak
'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0 , '|', 'Z', 'X', 'C', 'V', // 0x20 - 0x2f  //0x2a - lshift
'B', 'N', 'M', '<', '>', '?', 0  , '*', 0  , ' ', 0 ,  0 ,    0,   0,   0,   0, // 0x30 - 0x3f  //0x36 - rshift
  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40 - 0x4f
'2', '3', '0', '.',   0,   0,   0,   0                                          // 0x50 - 0x58
};

uint8_t keymap_ctrl[] =
{ 0,'\e', '1', '2', '3', '4', '5', RS, '7', '8', '9', '0', US, '=','\b', '\t',// 0x00 - 0x0f
DC1, ETB, ENQ, DC2, DC4, EM, NAK, HT, SI, DLE, ESC, GS, '\r',  0, SOH, DC3,    // 0x10 - 0x1f  //0x1d - brak
EOT, ACK, BEL, BS, LF, VT, FF, ';','\'', '`', 0 , FS, SUB, CAN, ETX, SYN,     // 0x20 - 0x2f  //0x2a - lshift
STX, SO, CR, ',', '.', DEL, 0  , '*', 0  , NUL, 0 ,  0 ,    0,   0,   0,   0, // 0x30 - 0x3f  //0x36 - rshift
  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40 - 0x4f
'2', '3', '0', '.',   0,   0,   0,   0                                          // 0x50 - 0x58
};

/// flagi dla klawiszy modyfikujących zachowanie pozostałych klawiszy
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

/// tablica z flagami oraz keycode'ami klawiszy modyfikujących
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
