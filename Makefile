SUBDIRS= sys
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
	cd image && qemu -fda floppy.img

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

commit: cleandepend
	svn commit
	make depend

burn: build-image
	sudo dd if=image/floppy.img of=${FLOPPY_DEV}

include ${IMPALA_MK}/subdir.mk
