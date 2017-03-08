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
    truncate\
    uname\
    sleep\
    vttest

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

.PHONY: all build build-image run distribution

all: build

build-image: ${IMAGE_FILE} build
	cp -f sys/kern/impala.gz image/root/boot/
	cd image && ./mtools.sh

run: build-image
	cd image && qemu-system-i386 -s -fda floppy.img #bochs -f impala-usr-local.bochs 


distribution: build
	@DIST_PROFILE="${DIST_PROFILE}" sh tools/distribution.sh

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

debug: build
	${GDB} -x tools/gdbscript sys/kern/impala

burn: build-image
	sudo dd if=image/floppy.img of=${FLOPPY_DEV}

include mk/subdir.mk

