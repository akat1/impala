#!/bin/sh
# $Id$

MDEL="mdel -i floppy.img"
MCOPY="mcopy -i floppy.img -o"
MMD="mmd -i floppy.img -o"
$MDEL -i floppy.img ::/BOOT/GRUB/MENU.LST
$MDEL -i floppy.img ::/BOOT/IMPALA.GZ
$MMD ::/sbin
$MMD ::/dev
#$MCOPY ../output/sbin/init ::/sbin/init
$MMD ::/bin
#$MCOPY ../output/bin/vttest ::/bin/vttest
$MCOPY -i floppy.img root/boot/impala.gz ::/BOOT
$MCOPY -i floppy.img root/boot/grub/menu.lst ::/BOOT/GRUB/MENU.LST

