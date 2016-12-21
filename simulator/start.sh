#!/bin/sh

mkdir -p test
if [ -z test/$1 ]; then
	echo "alreay exist $1"
else
	cp -ar temp/rootfs_x86_64 test/$1
fi
sudo mount --bind /dev test/$1/dev
sudo mount --bind /dev/pts test/$1/dev/pts
sudo mount --bind /proc test/$1/proc
sudo mount --bind /sys test/$1/sys
sudo mount -t tmpfs default test/$1/tmp
sudo env DM_ROOTDIR=/system/bin DM_IMEI=$2 DM_PHONE=$3 HOME=/home/root chroot test/$1 /bin/bash

sudo umount test/$1/tmp
sudo umount test/$1/sys
sudo umount test/$1/proc
sudo umount test/$1/dev/pts
sudo umount test/$1/dev
