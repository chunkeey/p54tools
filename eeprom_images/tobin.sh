#!/bin/sh

rm -rf binary tmp
mkdir binary tmp

for file in header/*.h; do
	base=`basename $file .h`

	echo "#include \"$file\"" > "tmp/$base.c"
	gcc -I`pwd` "tmp/$base.c" -c -o "tmp/$base.o"
	objcopy -O binary -j .rodata "tmp/$base.o" "binary/$base.bin"
done

rm -rf tmp
