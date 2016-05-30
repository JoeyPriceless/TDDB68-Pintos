#!/bin/bash

#Setup
#!!! Make sure linuxpintos folder is placed in home directory !!!
cd src/utils
make clean
ln -s $(which qemu-system-i386) qemu
make
chmod +x pintos
chmod +x pintos-mkdisk
chmod +x backtrace
chmod +x pintos-gdb
export PATH="${HOME}/linuxpintos/src/utils/:${PATH}/"
cd ../threads
make clean
make
cd build
# Will run test to make sure that the project is set up correctly.
pintos --qemu -- run alarm-multiple
