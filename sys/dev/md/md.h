/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __DEV_MD_H
#define __DEV_MD_H

#ifdef __KERNEL

int md_create(int unit, void *data, size_t s);
void md_destroy(int unit);
bool md_check(int unit);
void md_init(void);

#endif


#endif
