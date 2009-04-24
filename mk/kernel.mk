_KERNEL_BUILD= build
_KERNEL_CLEAN= clean
_KERNEL_DEPEND= depend
_KERNEL_CLEANDEPEND= cleandepend
.PHONY: ${KERNEL_MAIN}

C_FLAGS=-D__KERNEL ${C_FLAGS_}

LIBS?=
KERNEL?= kernel
LD_MAP?= ${KERNEL}.map
LD_SCRIPT?= ${IMPALA_SRCROOT}/sys/conf/kernel.ld
LIBDIR= -L ${IMPALA_SRCROOT}/sys/libkutil -L${IMPALA_SRCROOT}/sys/arch/${IMPALA_ARCH}/ -L ${IMPALA_SRCROOT}/sys/dev

include ${IMPALA_SRCROOT}/mk/build.mk
${_KERNEL_BUILD}: ${KERNEL}


${_KERNEL_CLEAN}:
	rm -f ${OBJS} ${KERNEL}


${KERNEL}: ${LIBDEPS} ${OBJS}
	@echo " LD ${KERNEL}"
	@${LD} ${LD_FLAGS} -o ${KERNEL} ${OBJS} ${LIBS}


${_LIB_CLEAN}:
	rm -f ${OBJS} ${KERNEL}

.depend ${_KERNEL_DEPEND}: ${SRCS}
	@rm -f .depend
	@for i in ${SRCS}; do	\
		${CC} ${C_FLAGS} -M $$i -MT `echo $$i | sed -e "s|\.c$$|\.o|g"`  >> .depend; \
	done;


${_KERNEL_CLEANDEPEND}:
	@echo "" > .depend


include .depend
