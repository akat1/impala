#ifndef __TAR_H
#define __TAR_H

#define TMAGIC "ustar"
#define TMAGLEN 6
#define TVERSION "00"
#define TGNUVERSION " "
#define TVERSLEN 2

#define REGTYPE     '0'
#define AREGTYPE    '\0'
#define LNKTYPE     '1'
#define SYMTYPE     '2'
#define CHRTYPE     '3'
#define BLKTYPE     '4'
#define DIRTYPE     '5'
#define FIFOTYPE    '6'
#define CONTTYPE    '7'

#define TOEXEC      0001
#define TOWRITE     0002
#define TOREAD      0004
#define TGEXEC      0010
#define TGWRITE     0020
#define TGREAD      0040
#define TUEXEC      0100
#define TUWRITE     0200
#define TUREAD      0400



#endif

