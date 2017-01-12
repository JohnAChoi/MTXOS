#! /bin/sh

VFD=mtximage

#cp mtximage2 mtximage

echo mount $VFD on /mnt
sudo mount -o loop $VFD /mnt
sudo cp mtx /mnt/boot
sudo umount /mnt

echo ready to go?
read dummy

# run QEMU on FDimage
qemu-system-i386 -fda $VFD -no-fd-bootchk -serial /dev/pts/5 -serial /dev/pts/6
