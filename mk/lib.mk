_LIB_BUILD?= build
_LIB_CLEAN?= clean
_LIB_DEPEND?= depend
_LIB_CLEANDEPEND?= cleandepend
_LIB_INSTALL?= install
LIBRARY?= libnoname

include ${IMPALA_SRCROOT}/mk/build.mk

${_LIB_BUILD}: ${LIBRARY}.a

${LIBRARY}.a: ${OBJS}
	@echo " AR ${LIBRARY}.a"
	${VERBOSE}${AR} r ${LIBRARY}.a ${OBJS}
	${VERBOSE}${RANLIB} ${LIBRARY}.a

${_LIB_CLEAN}:
	@echo " CLEAN"
	@rm -f ${OBJS} ${LIBRARY}.a

.depend ${_LIB_DEPEND}: ${SRCS}
	@rm -f .depend
	@for i in ${SRCS}; do	\
		${CC} ${_CFLAGS} -M $$i -MT `echo $$i | sed -e "s|\.c$$|\.o|g"`  >> .depend; \
	done;

${_LIB_CLEANDEPEND}:
	@echo "" > .depend

${_LIB_INSTALL}:
	@echo " CP ${LIBRARY}.a"
	${VERBOSE}cp ${LIBRARY}.a ${IMPALA_OUTPUT}/lib/

include .depend
