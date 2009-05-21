.SUFFIXES: .c.o .s.o .S.o

include ${IMPALA_MK}/config.mk

_BUILD_BUILD?= build
_BUILD_CLEAN?= clean
_BUILD_DEPEND?= depend

OBJS_1= ${SRCS:%.c=%.o}
OBJS_2= ${OBJS_1:%.s=%.o}
OBJS= ${OBJS_2}

.c.o:
	@echo " CC $<"
	${CC} -c ${_CFLAGS} -o $@ $<

.s.o:
	@echo " AS $<"
	@${AS} ${AS_FLAGS} -o $@ $<

