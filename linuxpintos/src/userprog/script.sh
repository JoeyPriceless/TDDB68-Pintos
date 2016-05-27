#!/bin/bash


#Lab 2
make -C ../examples/
make
cd build
rm fs.dsk
pintos-mkdisk fs.dsk 2
pintos --qemu -- -f -q
pintos --qemu -p ../../examples/lab2test -a lab2test -- -q
pintos --qemu -- run lab2test
#make check
