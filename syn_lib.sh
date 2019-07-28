#!/bin/sh

if [ "$#" -ne 1 ]; then
	echo "Please provide destination folder";
	exit 0;
fi

if [ ! -f $1/LibImport.txt ]; then
	echo "Unable to find LibImport.txt in destination folder.";
	exit 0;
fi

libs=`cat $1/LibImport.txt | dos2unix`

for lib in $libs; do
	rsync -avr --exclude build --exclude .settings --exclude .cproject --exclude .project --exclude main.cpp Library/$lib $1
done
