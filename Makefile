SUBDIRS= usr sys
IMAGE_FILE=image/floppy.img
IMAGE_FILE_=image/_floppy.img
FLOPPY_DEV?=/dev/fd0
.PHONY: all build build-image run init

all: build


build-image: ${IMAGE_FILE} build
	touch image/root/boot/impala
	cp image/root/boot/impala image/root/boot/impala.old
	cp sys/kern/impala image/root/boot/
	cd image && ./mtools.sh

run: build-image
	cd image && qemu -m 32 -fda floppy.img

init: build ${IMPALA_SRCROOT}/usr/sbin/init/init
	cp usr/sbin/init/init output/sbin/init
	cp usr/bin/test/test output/bin/test
	cd tools; gcc -std=c99 mfsutil.c -o mfsutil 
	./tools/mfsutil -i ./tools/root.image ./output
	cd misc; gcc -std=c99 -o toC toC.c
	./misc/toC tools/root.image > sys/kern/tmp_rootimage.c

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

commit:
	svn commit

burn: build-image
	sudo dd if=image/floppy.img of=${FLOPPY_DEV}

include mk/subdir.mk
