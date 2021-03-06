/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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
#include <sys/console.h>
#include <sys/vm.h>
#include <machine/interrupt.h>

/**
 * Funkcja wywoływana w sytuacjach awaryjnych.
 * Zatrzymuje system, wyświetlając podany komunikat.
 */

bool SYSTEM_DEBUG = FALSE;

void
panic(const char *msg, ...)
{
    splhigh();
    va_list ap;
    VA_START(ap, msg);
    kprintf("\npanic: ");
    vkprintf(msg, ap);
    kprintf("\n");
    VA_END(ap);
    while(1);
}

void
iprintf(const char *fmt, ...)
{
    int ipl = CIPL;
    spl0();
    va_list ap;
    VA_START(ap, fmt);
    vkprintf(fmt, ap);
    VA_END(ap);
    splx(ipl);
}

void
kprintf(const char *fmt, ...)
{
    va_list ap;
    VA_START(ap, fmt);
    vkprintf(fmt, ap);
    VA_END(ap);
}

void
vkprintf(const char *fmt, va_list ap)
{
    enum {
        KPRINTF_BUF = 512
    };
    char big_buf[KPRINTF_BUF];
    vsnprintf(big_buf, KPRINTF_BUF, fmt, ap);
    cons_msg(big_buf);
}


ssize_t
copyin(void *kaddr, const void *uaddr, size_t len)
{
    int err=0;
    if ((err=vm_is_avail((vm_addr_t)uaddr, len))) return err;
    memcpy(kaddr, uaddr, len);
    return len;
}

ssize_t
copyout(void *uaddr, const void *kaddr, size_t len)
{
    int err=0;
    if ((err=vm_is_avail((vm_addr_t)uaddr, len))) return err;
    memcpy(uaddr, kaddr, len);
    return len;
}


ssize_t
copyinstr(void *kaddr, const void *uaddr, size_t limit)
{
    int err=0;
    if ((err=vm_validate_string(uaddr, limit))<0) return err;
    strncpy(kaddr, uaddr, err+1);
    return 0;
}

ssize_t
copyoutstr(void *uaddr, const void *kaddr, size_t limit)
{
    return -1;
}

