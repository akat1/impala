#!/bin/sh
MDEL=mdel
MCOPY=mcopy

$MDEL -i floppy.img ::/BOOT/IMPALA /BOOT/IMPALA.OLD /BOOT/GRUB/MENU.LST
$MCOPY -i floppy.img root/boot/impala* ::/BOOT
$MCOPY -i floppy.img root/boot/grub/menu.lst ::/BOOT/GRUB/MENU.LST

