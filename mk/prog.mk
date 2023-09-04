_PROG_BUILD?= build
_PROG_CLEAN?= clean
_PROG_DEPEND?= depend
_PROG_CLEANDEPEND?= cleandepend
_PROG_INSTALL?= install
PREFIX=${ELF_PREFIX}
_CFLAGS=${_U_FLAGS}
PROG?= noname

include ${IMPALA_SRCROOT}/mk/build.mk

${_PROG_BUILD}: ${PROG}

${PROG}: ${OBJS} ${IMPALA_SRCROOT}/usr/lib/libc/libc.a ${IMPALA_SRCROOT}/usr/lib/crt/crt0.o
	@echo " LINK ${PROG}"
	${VERBOSE}${CC} -o ${PROG} ${_CFLAGS} ${OBJS} ${_U_LDFLAGS} ${LIBS}

${_PROG_CLEAN}:
	@echo " CLEAN"
	@rm -f ${OBJS} ${PROGRAM}

.depend ${_PROG_DEPEND}: ${SRCS}
	@rm -f .depend
	@for i in ${SRCS}; do	\
		${CC} ${_CFLAGS} ${CFLAGS} -M $$i -MT `echo $$i | sed -e "s|\.c$$|\.o|g"`  >> .depend; \
	done;

${_PROG_CLEANDEPEND}:
	@echo "" > .depend

${_PROG_INSTALL}:
	@echo " CP ${PROGRARY}.a"
	@cp ${PROGRARY}.a ${IMPALA_OUTPUT}/lib/

include .depend
