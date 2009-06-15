_KERNEL_BUILD= build
_KERNEL_CLEAN= clean
_KERNEL_DEPEND= depend
_KERNEL_CLEANDEPEND= cleandepend
.PHONY: ${KERNEL_MAIN}

_CFLAGS=-D__KERNEL ${_K_FLAGS}

LIBS?=
KERNEL?= kernel
LD_MAP?= ${KERNEL}.map
LD_SCRIPT?= ${IMPALA_SRCROOT}/sys/conf/kernel.ld
LIBDIR= -L ${IMPALA_SRCROOT}/sys/libkutil -L${IMPALA_SRCROOT}/sys/arch/${IMPALA_ARCH}/ -L ${IMPALA_SRCROOT}/sys/dev -L ${IMPALA_SRCROOT}/sys/fs

include ${IMPALA_SRCROOT}/mk/build.mk
${_KERNEL_BUILD}: ${KERNEL}


${_KERNEL_CLEAN}:
	@rm -f ${OBJS} ${KERNEL}


tmp_rootimage.c:
	cp _tmp_rootimage.c tmp_rootimage.c

${KERNEL}: tmp_rootimage.c ${LIBDEPS} ${OBJS} ${LD_SCRIPT}
	@echo " LD ${KERNEL} (compressed)"
	@${LD} ${LD_FLAGS} -o ${KERNEL} ${OBJS} ${LIBS}
	@${GZIP} -f -c ${KERNEL} > ${KERNEL}.gz


${_LIB_CLEAN}:
	@echo " CLEAN"
	@rm -f ${OBJS} ${KERNEL}

.depend ${_KERNEL_DEPEND}: ${SRCS}
	@rm -f .depend
	@for i in ${SRCS}; do	\
		${CC} ${_CFLAGS} -M $$i -MT `echo $$i | sed -e "s|\.c$$|\.o|g"`  >> .depend; \
	done;


${_KERNEL_CLEANDEPEND}:
	@echo "" > .depend


include .depend
