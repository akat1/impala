SUBDIRS= sys usr
IMAGE_FILE=image/floppy.img
IMAGE_FILE_=image/_floppy.img
FLOPPY_DEV?=/dev/fd0
GDB?=gdb
DISTDIR=output/dist
SPECDIR=output/impala
.PHONY: all build build-image run init

all: build

build-image: ${IMAGE_FILE} build
	cp -f sys/kern/impala.gz image/root/boot/
	cd image && ./mtools.sh

run: build-image
	cd image && qemu -s -fda floppy.img #bochs -f impala-usr-local.bochs 

dist: init
init: build ${IMPALA_SRCROOT}/usr/sbin/init/init
	cp COPYRIGHT ${DISTDIR}/
	cp usr/sbin/init/init ${DISTDIR}/sbin/init
	cp usr/sbin/ttyvrun/ttyvrun ${DISTDIR}/sbin/ttyvrun
#	cp usr/sbin/mount/mount ${DISTDIR}/sbin/mount
#	cp usr/bin/test/test ${DISTDIR}/bin/test
	cp usr/bin/vttest/vttest ${DISTDIR}/bin/vttest
	cp usr/bin/sh/sh ${DISTDIR}/bin/sh
	cp usr/bin/ls/ls ${DISTDIR}/bin/ls
	cp usr/bin/ps/ps ${DISTDIR}/bin/ps
	cp usr/bin/uname/uname ${DISTDIR}/bin/uname
	cp usr/bin/cat/cat ${DISTDIR}/bin/cat
	cp usr/etc/rc.* ${DISTDIR}/etc/
	cp usr/etc/motd ${DISTDIR}/etc/
	cp usr/bin/tar/tar ${SPECDIR}/tar
#i	cp usr/bin/gzip/gzip ${SPECDIR}/gzip
	cp usr/sbin/preinit/preinit ${SPECDIR}/preinit
	cd output/dist && tar cvf ../impala/dist.tar *

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

commit:
	svn commit

update:
	svn log -r BASE:HEAD > CHANGES
	svn update && cat CHANGES

install_sdk:
	ln -sf ${IMPALA_SRCROOT}/sys/sys usr/lib/libc/include/
	ln -sf ${IMPALA_SRCROOT}/sys/arch/x86/machine usr/lib/libc/include/
	sudo ln -sf ${IMPALA_SRCROOT}/usr/lib/libc/include ${AOUT_PATH}/
	sudo ln -sf ${IMPALA_SRCROOT}/usr/lib/libc/libc.a ${AOUT_PATH}/lib/
	sudo ln -sf ${IMPALA_SRCROOT}/usr/lib/crt/crt0.o ${AOUT_PATH}/lib/
	sudo ln -sf ${IMPALA_SRCROOT}/usr/lib/libpthread/libpthread.a ${AOUT_PATH}/lib/

debug: build
	${GDB} -x tools/gdbscript sys/kern/impala

burn: build-image
	sudo dd if=image/floppy.img of=${FLOPPY_DEV}

include mk/subdir.mk

