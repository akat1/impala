SUBDIRS= sys usr
IMAGE_FILE=image/floppy.img
IMAGE_FILE_=image/_floppy.img
FLOPPY_DEV?=/dev/fd0
.PHONY: all build build-image run init

all: build


build-image: ${IMAGE_FILE} build
	cp -f sys/kern/impala.gz image/root/boot/
	cd image && ./mtools.sh

run: build-image
	cd image && qemu -s -m 32 -fda floppy.img # bochs -f impala-usr-local.bochs && qemu -s -m 32 -fda floppy.img 

init: build ${IMPALA_SRCROOT}/usr/sbin/init/init
	cp COPYRIGHT output
	cp usr/sbin/init/init output/sbin/init
	cp usr/bin/test/test output/bin/test
	cp usr/bin/vttest/vttest output/bin/vttest
	cp usr/bin/sh/sh output/bin/sh
	cd tools; gcc -std=c99 mfsutil.c -o mfsutil 
	./tools/mfsutil -i ./tools/root.image ./output
	cd misc; gcc -std=c99 -o toC toC.c
	./misc/toC tools/root.image > sys/kern/tmp_rootimage.c

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

commit:
	svn commit

update:
	svn log -r BASE:HEAD > CHANGES
	svn update && cat CHANGES

debug: build
	gdb -x tools/gdbscript sys/kern/impala

burn: build-image
	sudo dd if=image/floppy.img of=${FLOPPY_DEV}

include mk/subdir.mk

