SUBDIRS= sys usr
IMAGE_FILE=image/floppy.img
IMAGE_FILE_=image/_floppy.img
GDB?=gdb

DISTDIR=output/dist
SPECDIR=output/impala

.PHONY: all build distribution

all: build

distribution: build
	@DIST_PROFILE="${DIST_PROFILE}" sh tools/distribution.sh

${IMAGE_FILE}: ${IMAGE_FILE_}
	@cp ${IMAGE_FILE_} ${IMAGE_FILE}

debug: build
	${GDB} -x tools/gdbscript sys/kern/impala

include mk/subdir.mk
