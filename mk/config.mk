.SUFFIXES: .c.o .s.o .S.o
IMPALA_USR=${IMPALA_SRCROOT}/usr
PREFIX?=${ELF_PREFIX}
ELF_PREFIX?=
ELF_CC=${ELF_PREFIX}gcc
AOUT_PREFIX?=i386-aout-
AOUT_CC=${AOUT_PREFIX}gcc
__INCDIR= -I ${IMPALA_SRCROOT}/sys -I ${IMPALA_SRCROOT}/sys/arch/${IMPALA_ARCH}/ 
_INCDIR?=${__INCDIR} ${INCDIR}
# XXXamd64: ADD -m32 for x86
__C_FLAGS=-std=c99 -ffreestanding -Wall -Wstrict-prototypes\
	-Wmissing-prototypes ${_INCDIR} -nostdlib -D__Impala__  
# XXXamd64
_K_FLAGS=-mcmodel=large -z max-page-size=0x1000 -mno-red-zone ${__C_FLAGS} -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow\
    -D__KERNEL -Werror -nostdinc -g -fno-inline  
_U_FLAGS=-m32 ${__C_FLAGS} ${__INCDIR} -I${IMPALA_USR}/lib/libc/include\
        -I${IMPALA_USR}/lib/libpthread/include\
        -I${IMPALA_USR}/lib/libz/ 
_U_LDFLAGS=-Wl,-e,__start -T${IMPALA_USR}/conf/user.ld -nostdlib\
        -L${IMPALA_USR}/lib/libc -L${IMPALA_USR}/lib/libpthread\
        -L${IMPALA_USR}/lib/libz -L${IMPALA_USR}/lib/crt
_CFLAGS?= ${_K_FLAGS_} ${CFLAGS}
CC= ${PREFIX}gcc
AS= ${PREFIX}as
# XXXamd64
# AS_FLAGS= -32 
AR=${PREFIX}ar
GZIP=gzip -9
STRIP=${PREFIX}strip
AR=${PREFIX}ar
RANLIB=${PREFIX}ranlib
LD=${PREFIX}ld
LIBDIR?=
LD_FLAGS=-z max-page-size=0x1000 -nostdlib -T ${LD_SCRIPT} --cref -Map ${LD_MAP} ${LIBDIR}
INSTALL=cp
AWK=awk
VERBOSE?=@
