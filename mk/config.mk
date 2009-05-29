.SUFFIXES: .c.o .s.o .S.o
IMPALA_USR=${IMPALA_SRCROOT}/usr
PREFIX?=${ELF_PREFIX}
ELF_PREFIX?=
ELF_CC=${ELF_PREFIX}gcc
AOUT_PREFIX?=i386-aout-
AOUT_CC=${AOUT_PREFIX}gcc
__INCDIR= -I ${IMPALA_SRCROOT}/sys -I ${IMPALA_SRCROOT}/sys/arch/${IMPALA_ARCH}/ 
_INCDIR?=${__INCDIR} ${INCDIR}
__C_FLAGS=-m32 -std=c99 -ffreestanding -nostdinc -Wall -Wstrict-prototypes\
	-Wmissing-prototypes -Werror ${_INCDIR} -nostdlib
_K_FLAGS=${__C_FLAGS} -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow\
    -D__KERNEL
_U_FLAGS=${__C_FLAGS} ${__INCDIR} -I${IMPALA_SRCROOT}/usr/lib/libc/include
_U_LDFLAGS=-Wl,-e,__start -L${IMPALA_USR}/lib/libc ${IMPALA_USR}/lib/crt/crt0.o -lgcc -lc
_CFLAGS?= ${_K_FLAGS_} ${CFLAGS}
CFLAGS?=${_CFLAGS}
CC= ${PREFIX}gcc
AS= ${PREFIX}as
AS_FLAGS= -32
AR=${PREFIX}ar
STRIP=${PREFIX}strip
AR=${PREFIX}ar
RANLIB=${PREFIX}ranlib
LD=${PREFIX}ld
LIBDIR?=
LD_FLAGS=-nostdlib -T ${LD_SCRIPT} --cref -Map ${LD_MAP} ${LIBDIR}
INSTALL=cp

