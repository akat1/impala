SUBDIRS= sys usr
IMAGE_FILE=image/floppy.img
IMAGE_FILE_=image/_floppy.img
FLOPPY_DEV?=/dev/fd0
.PHONY: all build build-image run

all: build


build-image: ${IMAGE_FILE} build
	touch image/root/boot/impala
	cp image/root/boot/impala image/root/boot/impala.old
	cp sys/kern/impala image/root/boot/
	cd image && ./mtools.sh

run: build-image
	cd image && qemu -m 32 -fda floppy.img

init: build
	cd tools; g++ mfsutil.c -o mfsutil -O2;
	./tools/mfsutil ./output -i ./tools/root.image
	cd misc; gcc -std=c99 -o toC toC.c
	./misc/toC tools/root.image > sys/kern/tmp_rootimage.c

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

commit:
	svn commit

burn: build-image
	sudo dd if=image/floppy.img of=${FLOPPY_DEV}

include mk/subdir.mk
