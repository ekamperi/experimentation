#!/bin/sh
TESTDEV=/home/stathis/eleutheria/cdev
# Copy dev files to netbsd source tree
cp -v mydev.c /usr/src/sys/dev
cp -v mydev.h /usr/src/sys/sys

# Recompile kernel
cd /usr/src
./build.sh  -O ../obj -T ../tools -u kernel=XEN3_DOMU || exit

# Build testdev
cd $TESTDEV
pwd
gcc testdev.c -o testdev -lprop -I /usr/src/sys -Wall

# Shutdown domU if it's running
domUid=`xm list | grep nbsd-dom2 | awk {'print $2'}`
xm shutdown $domUid

# Copy testdev in domU's virtual disk
sleep 2
vnconfig vnd0 /usr/xen/nbsd-disk || exit
mount /dev/vnd0a /mnt
cp /home/stathis/eleutheria/cdev/testdev /mnt/root
umount /dev/vnd0a
vnconfig -u vnd0
