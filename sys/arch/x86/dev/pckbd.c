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
#include <sys/utils.h>
#include <machine/interrupt.h>
#include <machine/pckbd.h>
#include <machine/pckbd_tables.h>
#include <machine/io.h>


enum {
    PCKBD_DATA_PORT = 0x60,
    PCKBD_STATUS_PORT = 0x64,
    PCKBD_COMMAND_PORT = 0x64
};

enum {
    MAX_SC_LINEAR = 88,
    PCKBD_BUFSIZE = 20,
};

//static int bh_in_queue=0;
int key_down[256];      ///< wykaz naci¶niêtych klawiszy, indeksowany po 'keycode'
                        /// pewno lepiej bêdzie indeksowaæ po czym¶ innym ;)
char key_modifiers;     /// wykaz naci¶niêtych klawiszy-modyfikatorów

static bool i8042_irq1(void);
static void __enqueue_keycode(int sc);
static void set_modifiers(void);


void
__enqueue_keycode(int kc)
{
    char c = 0;
    // sprawdzamy czy mamy miejsce w kolejce.
    bool shift=(key_modifiers & (KM_LSHIFT | KM_RSHIFT))>0;
    bool ctrl =(key_modifiers & (KM_LCONTROL | KM_RCONTROL))>0;
    if(kc<MAX_SC_LINEAR) {
        if(ctrl) {
            c = keymap_ctrl[kc];
        } else {
            c = keymap_normal[kc];
            if('a' <= c && c <= 'z')
                shift ^= (key_modifiers & KM_CAPSLOCK)>0;

            if(shift)
                c = keymap_shift[kc];
        }
    } else if(kc == DEL)
        c = DEL;
    else {
        char *x = keymap_string[kc-128];
        if(x) {
            cons_input_char(0233);
            cons_input_string(x);
        }
        return;
    }
    if(c || (ctrl && kc==' '))
        cons_input_char(c);
}

void
set_modifiers()
{
    for(int i=0; i<8; i++)
        if(key_down[key_modifiers_tab[i][1]])
            key_modifiers |= key_modifiers_tab[i][0];
        else
            key_modifiers &= ~key_modifiers_tab[i][0];
}

void
pckbd_init()
{
    irq_install_handler(IRQ1, i8042_irq1, IPL_TTY);
}

bool
i8042_irq1()
{
    static uint8_t last_scancode = 0;
    uint8_t scancode = io_in8(PCKBD_DATA_PORT);

    uint8_t up_action = scancode & 0x80;
    uint8_t keycode = 0;


    if(last_scancode == 0 && (scancode == 0xe0 || scancode == 0xe1)) {
        last_scancode = scancode;
        irq_done();
        return TRUE;
    }

    scancode &= 0x7f;

    if(last_scancode == 0) {
        if (scancode <= MAX_SC_LINEAR) {
            keycode = scancode;
        } else {
            ///@todo inne klawisze...
        }
    } else if(last_scancode == 0xe0) {
        last_scancode = 0;
        if(e0_kcodes[scancode] == 0) {
//             kprintf("WARNING: Unknown keyboard scancode 0xe0%x\n", scancode);
        } else {
            keycode = e0_kcodes[scancode];
        }
    } else if(last_scancode == 0xe1) {
        //To raczej 'pause break'...
        if(scancode == 0x45 && up_action) {  //ostatni bajt scancode klawisza pause break
            last_scancode = 0;
            keycode = KC_PAUSE;
        }
    }

    if(keycode) {
        if(keycode == KC_CAPSLOCK) {
            if(!up_action)
                key_down[KC_CAPSLOCK] ^= TRUE;
        } else if(up_action)
            key_down[keycode] = FALSE;
        else {
            key_down[keycode] = TRUE;
            __enqueue_keycode(keycode);
        }
        set_modifiers();
    }

    irq_done();
    return TRUE;
}

void
set_kbd_repeat_rate(uint rate)
{
}

void
set_kbd_delay(uint delay)
{
}

