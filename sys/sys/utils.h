/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_UTILS_H
#define __SYS_UTILS_H


void
panic(const char* msg, ...);

#define KASSERT(x) if(!(x)) \
    panic("Assertion ( "  #x  " ) failed in file: " __FILE__ ":%u, in function: %s", __LINE__,  __func__);

#endif

