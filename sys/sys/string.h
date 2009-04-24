/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_STRING_H
#define __SYS_STRING_H

#ifdef __KERNEL

addr_t mem_move(addr_t dst, addr_t src, size_t len);
addr_t mem_cpy(addr_t dst, addr_t src, size_t len);
addr_t mem_set(addr_t s, char c, size_t len);
size_t str_len(const char *s);
int str_cmp(const char *a, const char *b);
int snprintf(char *dst, size_t size, const char *fmt, ...);
int vsnprintf(char *dst, size_t size, const char *fmt, va_list ap);


#define mem_zero(s, l) mem_set(s, 0, l)

#endif

#endif
