/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_KPRINTF_H
#define __SYS_KPRINTF_H

#include <machine/video.h>

/* Tymczasowy interfejs do rysowania po ekranie */

void kprintf(const char *fmt, ...);
void vkprintf(const char *fmt, va_list ap);

#include <sys/sched.h>

#define TRACE_ATTR COLOR_BRIGHTBLUE

#define TRACE_IN(fmt, args...)\
    do { \
        textscreen_enable_forced_attr(TRACE_ATTR);\
        kprintf("@ %s (", __func__);\
        kprintf(fmt, ## args);\
        kprintf(")\n");\
        for (unsigned int xxx = 0; xxx < 0xfffff; xxx++);\
        textscreen_disable_forced_attr();\
    } while (0);
    
#define TRACE_IN0() TRACE_IN("");

#endif

