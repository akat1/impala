_LIB_BUILD= build
_LIB_CLEAN= clean
_LIB_DEPEND= depend

LIBRARY?= libnoname

include ${IMPALA_SRCROOT}/mk/build.mk

${_LIB_BUILD}: ${LIBRARY}.a

${LIBRARY}.a: ${OBJS}
	@echo "> archiving ${LIBRARY}.a"
	@${AR} r ${LIBRARY}.a ${OBJS}
	@${RANLIB} ${LIBRARY}.a

${_LIB_CLEAN}:
	rm -f ${OBJS} ${LIBRARY}.a

${_LIB_DEPEND}:
	@${CC} ${C_FLAGS} -M ${SRCS} > .depend

