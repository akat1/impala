SUBDIRS= sys

build-image: build
	touch image/root/boot/impala
	cp image/root/boot/impala image/root/boot/impala.old
	cp sys/kern/impala image/root/boot/
	cd image && sudo ./`uname`.sh

run: build-image
	cd image && qemu -fda floppy.img


include ${IMPALA_MK}/subdir.mk
