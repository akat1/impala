#ifndef __SYS_KPRINTF_H
#define __SYS_KPRINTF_H

#include <machine/video.h>

/* Tymczasowy interfejs do rysowania po ekranie */

void kprintf(const char *fmt, ...);
void vkprintf(const char *fmt, va_list ap);

#define TRACE_ATTR COLOR_BRIGHTBLUE

#define TRACE_IN(fmt, args...)\
    do { \
        textscreen_enable_forced_attr(TRACE_ATTR);\
        kprintf("@ %s (", __func__);\
        kprintf(fmt, ## args);\
        kprintf(")\n");\
        textscreen_disable_forced_attr();\
    } while (0);
    
#define TRACE_IN0() TRACE_IN("");

#endif

