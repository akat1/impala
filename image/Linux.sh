#!/bin/sh

if [ $UID -ne 0 ]; then
	echo "Only root can do that..."
	exit 0
fi

set -x

mount -o loop floppy.img fd
mv root/boot/impala root/boot/impala.old
cp ../kernel/impala root/boot/impala
cp root/boot/impala* fd/boot/
cp root/boot/grub/menu.lst fd/boot/grub/
sync
umount fd
