
# Na podstawie mekfile.org, wygenerowanego automatycznie przez configure z makefile.in.
# Id: makefile.in,v 1.36 2005/10/19 22:50:13 tom Exp 

# $Id$

PROG = vttest
RELEASE = 2
PREFIX=${AOUT_PREFIX}
CFLAGS=-DHAVE_CONFIG_H -DRELEASE=2 -I. -I.  -U_XOPEN_SOURCE -D_XOPEN_SOURCE=500 -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=199506L

C_SRC = \
	charsets.c \
	color.c \
	draw.c \
	esc.c \
	keyboard.c \
	main.c \
	mouse.c \
	nonvt100.c \
	printer.c \
	reports.c \
	reset.c \
	setup.c \
	sixel.c \
	status.c \
	tek4014.c \
	ttymodes.c \
	unix_io.c \
	vt220.c \
	vt320.c \
	vt420.c \
	vt52.c \
	xterm.c

SRCS = $(C_SRC) 

include ${IMPALA_MK}/prog.mk
