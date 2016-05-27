

#!/bin/bash
export PATH="${HOME}/linuxpintos/src/utils/:${PATH}/"

TEST_FILE=$1
if [ $TEST_FILE ]
then
	echo "Building"
	make
	echo "Creating disk"
	pintos-mkdisk build/fs.dsk 2
	echo "Executing"
	pintos --qemu -p ../examples/$TEST_FILE -a $TEST_FILE -- -f -q
	pintos --qemu -- run $TEST_FILE
	echo "Done"
else
	echo "Please input testfile"
fi


