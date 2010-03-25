#!/bin/sh
# $Id$

if [ ! -e "floppy.img" ]; then
	echo "ERROR: floppy.img does not exist! Exiting..."
	exit 1
fi

MDEL="mdel -i floppy.img"
MDELTREE="mdeltree -i floppy.img"
MCOPY="mcopy -s -i floppy.img -o"
MMD="mmd -i floppy.img -o"
$MDEL ::/boot/grub/menu.lst
$MDEL ::/boot/impala.gz
$MDELTREE ::/impala
$MCOPY root/boot/impala.gz ::/boot
$MCOPY root/boot/grub/menu.lst ::/boot/grub/menu.lst
$MCOPY ../output/impala ::/
$MCOPY README.txt ::/
$MCOPY ../COPYRIGHT ::/COPYRIGHT.txt
