/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest objêty licencj±, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321


#include <machine/endian.h>

#if 0
#define BYTE_ORDER __BYTE_ORDER

#define LITTLE_ENDIAN LITTLE_ENDIAN
#define BIG_ENDIAN BIG_ENDIAN
#endif

#endif
