_LIB_BUILD= build
_LIB_CLEAN= clean
_LIB_DEPEND= depend
_LIB_CLEANDEPEND= cleandepend

LIBRARY?= libnoname

include ${IMPALA_SRCROOT}/mk/build.mk

${_LIB_BUILD}: ${LIBRARY}.a

${LIBRARY}.a: ${OBJS}
	@echo "> archiving ${LIBRARY}.a"
	@${AR} r ${LIBRARY}.a ${OBJS}
	@${RANLIB} ${LIBRARY}.a

${_LIB_CLEAN}:
	rm -f ${OBJS} ${LIBRARY}.a

.depend ${_LIB_DEPEND}: ${SRCS}
	@rm -f .depend
	@for i in ${SRCS}; do	\
		${CC} ${C_FLAGS} -M $$i -MT `echo $$i | sed -e "s|\.c$$|\.o|g"`  >> .depend; \
	done;

${_LIB_CLEANDEPEND}:
	@echo "" > .depend

include .depend
