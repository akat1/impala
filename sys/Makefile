SUBDIRS= arch libkutil dev fs kern 

install: build
	@echo " CLEAN sys machine"
	@rm -fr ${IMPALA_OUTPUT}/usr/include/sys ${IMPALA_OUTPUT}/usr/machine
	@echo " CP sys"
	@cp -r sys ${IMPALA_OUTPUT}/usr/include/
	@echo " CP machine"
	@cp -r arch/${IMPALA_ARCH}/machine ${IMPALA_OUTPUT}/usr/
	@echo " CP kern/impala"
	@cp kern/impala ${IMPALA_OUTPUT}/boot/

_SUBDIR_INSTALL=__subdir_install
include ${IMPALA_MK}/subdir.mk

