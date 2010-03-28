/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest objęty licencją, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#ifndef __MNTENT_H__
#define __MNTENT_H__

// chwilowy zrzut z glibc
// chcemy mieć linuksowy interfejs do tego, czy jakiś inny / własny?

struct mntent {
    char *mnt_fsname;
    char *mnt_dir;
    char *mnt_type;
    char *mnt_opts;
//    int mnt_freq;
//    int mnt_passno;
};


FILE *setmntent (const char *file, const char *mode);

struct mntent *getmntent (FILE *stream);


#endif