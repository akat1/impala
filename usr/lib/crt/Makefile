PREFIX=${AOUT_PREFIX}
CFLAGS=${_U_FLAGS} -I${IMPALA_SRCROOT}/usr/lib/libc/private/
include ${IMPALA_SRCROOT}/mk/build.mk
SRCS=\
	crt0.c

build: ${OBJS}

install:
	${INSTALL} crt0.o ${IMPALA_OUTPUT}/lib/

depend:
cleandepend:

clean:
	@rm -f ${OBJS}

