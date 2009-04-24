.SUFFIXES: .c.o .s.o .S.o
GCC_PREFIX?=
BINUTILS_PREFIX?=
CC=${GCC_PREFIX}gcc
INCDIR= -I ${IMPALA_SRCROOT}/sys -I ${IMPALA_SRCROOT}/sys/arch/${IMPALA_ARCH}/ 
C_FLAGS_=-m32 -std=c99 -ffreestanding -nostdinc -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror ${INCDIR}\
	 -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
C_FLAGS?= ${C_FLAGS_}

AS= ${BINUTILS_PREFIX}as
AS_FLAGS= -32

AR=${BINUTILS_PREFIX}ar

STRIP=${BINUTILS_PREFIX}strip
AR=${BINUTILS_PREFIX}ar
RANLIB=${BINUTILS_PREFIX}ranlib

LD=${BINUTILS_PREFIX}ld
LIBDIR?=
LD_FLAGS=-nostdlib -T ${LD_SCRIPT} --cref -Map ${LD_MAP} ${LIBDIR}



