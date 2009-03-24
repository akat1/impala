/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/kprintf.h>
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
    MAX_SC_LINEAR = 88
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
    char c = '?';
    bool shift=(key_modifiers & (KM_LSHIFT | KM_RSHIFT))>0;
    if(kc<MAX_SC_LINEAR) {
        c = keymap_normal[kc];
        if('a' <= c && c <= 'z')
            shift ^= (key_modifiers & KM_CAPSLOCK)>0;
        
        if(shift)
            c = keymap_shift[kc];
    }

    if(c=='Q')
        panic("OH NOOOO..... you pressed Q .... u r stupitt!!!\n");
    KASSERT(c!='W');
    
    kprintf("%c", c);
    //put_to_console_queue(c);  //czy co¶ ko³o - jak kto¶ potrzebuje
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
    irq_install_handler(IRQ1, i8042_irq1);
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
            //TODO: inne klawisze...
        }
    } else if(last_scancode == 0xe0) {
        last_scancode = 0;
        if(e0_kcodes[scancode] == 0) {
            kprintf("WARNING: Unknown keyboard scancode 0xe0%x\n", scancode);
        } else {
            keycode = e0_kcodes[scancode];
        }
    } else if(last_scancode == 0xe1) {
        //To raczej 'pause break'...
        if(scancode == 0x45 && up_action) {  //ostatni bajt scancode klawisza pause break
            last_scancode = 0;
            #define KC_PAUSE_BREAK 139  //tak tymczasowo ;p
            keycode = KC_PAUSE_BREAK;
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
/*    static tq_task_t bh_task = {0, &__enqueue_keycode, 0};
    if(!bh_in_queue)
        tq_enqueue(&bh_task);
    bh_in_queue = 1;*/
     
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

