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
#In order for this line to work, open at least two terminals.
#Use the tty command to find out which port those terminals are. 
#Replace the "/dev/pts/5" and "/dev/pts/6" with the output of tty on each terminal
qemu-system-i386 -fda $VFD -no-fd-bootchk -serial /dev/pts/5 -serial /dev/pts/6
