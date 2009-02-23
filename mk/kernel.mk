_KERNEL_BUILD= build
_KERNEL_CLEAN= clean
_KERNEL_DEPEND= depend

C_FLAGS=-D__KERNEL ${C_FLAGS_}

LIBS?=
KERNEL?= kernel
LD_MAP?= ${KERNEL}.map
LD_SCRIPT?= ${IMPALA_SRCROOT}/sys/conf/kernel.ld
LIBDIR= -L ${IMPALA_SRCROOT}/sys/libkutil -L${IMPALA_SRCROOT}/sys/arch/${IMPALA_ARCH}/

include ${IMPALA_SRCROOT}/mk/build.mk
${_KERNEL_BUILD}: ${KERNEL}


${_KERNEL_CLEAN}:
	rm -f ${OBJS} ${KERNEL}


${KERNEL}: ${LIBDEPS} ${OBJS}
	@echo "> linking ${KERNEL}"
	@${LD} ${LD_FLAGS} -o ${KERNEL} ${OBJS} ${LIBS}


${_LIB_CLEAN}:
	rm -f ${OBJS} ${KERNEL}

