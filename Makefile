SUBDIRS= sys usr
IMAGE_FILE=image/floppy.img
IMAGE_FILE_=image/_floppy.img
FLOPPY_DEV?=/dev/fd0
GDB?=gdb

DISTDIR=output/dist
SPECDIR=output/impala
PROG_BIN=\
    cat\
    ls\
    mkdir\
    minigzip\
    ps\
    sh\
    tar\
#    truncate\
    #uname\
#    sleep\
#    vttest

PROG_SBIN=\
    init\
    login\
    ttyvrun

DEMOS_BIN=
A=\
    pthdemo1\
    pthdemo2\
    pthdemo3\
    sysvmsg\
    pfault\
    signal\
    pipedemo

DISTDIRS=\
    output/dist/bin\
    output/dist/sbin\
    output/dist/etc\
    output/dist/demos\
    output/impala\
    output/dist/var/tmp\
    output/dist/tmp

.PHONY: all build build-image run init

all: build

build-image: ${IMAGE_FILE} build
	cp -f sys/kern/impala.gz image/root/boot/
	cd image && ./mtools.sh

run: build-image
	cd image && qemu -s -fda floppy.img #bochs -f impala-usr-local.bochs 

dist: init
init: build ${IMPALA_SRCROOT}/usr/sbin/init/init
	rm -rf output
	mkdir -p ${DISTDIRS}
	cp COPYRIGHT ${DISTDIR}/
	for prog in ${PROG_BIN}; do cp usr/bin/$$prog/$$prog ${DISTDIR}/bin/$$prog; done
	for prog in ${PROG_SBIN}; do cp usr/sbin/$$prog/$$prog ${DISTDIR}/sbin/$$prog; done
	for prog in ${DEMOS_BIN}; do cp usr/demos/$$prog/$$prog ${DISTDIR}/demos/$$prog; done
	cp usr/etc/rc.* ${DISTDIR}/etc/
	cp usr/etc/motd ${DISTDIR}/etc/
	cp usr/etc/profile ${DISTDIR}/etc/
	cp usr/bin/tar/tar ${SPECDIR}/tar
	cp usr/bin/minigzip/minigzip ${SPECDIR}/minigzip
	cp usr/sbin/preinit/preinit ${SPECDIR}/preinit
	#cp usr/bin/vi/build/nvi ${DISTDIR}/bin/vi
	#cp usr/bin/vi/build/nex ${DISTDIR}/bin/ex
	cd output/dist && tar -cvf ../impala/syspack.tar *
	gzip -9 output/impala/syspack.tar

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

commit:
	if [ -e COMMIT ];\
	then\
		svn commit -F COMMIT && rm COMMIT;\
	else\
		svn commit;\
	fi

update:
	svn update
	svn log -r BASE:HEAD

log:
	svn log -r BASE:HEAD

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

