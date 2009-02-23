#!/bin/sh

mdconfig -a -t vnode -f floppy.img -u 4
mount_msdosfs /dev/md4 fd
cp root/boot/impala* fd/boot/
cp root/boot/grub/menu.lst fd/boot/grub/
umount fd
mdconfig -d -u 4

