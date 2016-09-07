#!/bin/bash
# ska ligga i src/userprog/
make clean ../examples/
make clean ../threads/
make clean ../filesys/
make -C ../examples/
make -C ../threads/
make -C ../filesys/
make clean
make
cd build
rm fs.dsk
pintos-mkdisk fs.dsk 2
pintos --qemu -v -- -f -q
pintos --qemu -v -p ../../examples/pfs -a pfs -- -q
pintos --qemu -v -p ../../examples/pfs_reader -a pfs_reader -- -q
pintos --qemu -v -p ../../examples/pfs_writer -a pfs_writer -- -q

i="0"

while [ $i -lt 5 ]
do
	pintos --qemu -v -- -q run pfs > /dev/null
	pintos --qemu -v -g messages -- -q > /dev/null
	grep -c cool messages
	rm messages
	pintos --qemu -v -- -q rm messages > /dev/null
	i=$[$i+1]
done
