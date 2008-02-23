#!/bin/sh
# Copy dev files to netbsd source tree
sudo cp -v mydev.c /usr/src/sys/dev
sudo cp -v mydev.h /usr/src/sys/sys

# Recompile kernel
cd /usr/src
sudo ./build.sh  -O ../obj -T ../tools -u kernel=MY_GENERIC

# Install kernel
cd ../obj/sys/arch/i386/compile/MY_GENERIC/
sudo make install

# Build testdev
cd ~/eleutheria/netbsd/cdev
gcc testdev.c -lprop -I /usr/src/sys -Wall

