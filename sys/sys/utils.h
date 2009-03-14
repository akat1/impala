/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_UTILS_H
#define __SYS_UTILS_H


/**
 * Funkcja wywo³ywana w sytuacjach awaryjnych.
 * Zatrzymuje system, wy¶wietlaj±c podany komunikat.
 */

void
panic(const char* msg);

#endif

