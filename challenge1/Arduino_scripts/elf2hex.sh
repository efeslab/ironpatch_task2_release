#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "Usage: $0 <file_name.ino.elf>"
fi

INPUT=$1
OUTPUT=$(echo $INPUT | sed -e 's/elf$/hex/')

if ! [[ $INPUT =~ \.ino\.elf$ ]];
then
	echo "Bad extension"
	echo "Usage: $0 <file_name.ino.elf>"
	exit 1
fi

echo "$0: input: $INPUT"
echo "$0: output: $OUTPUT"
avr-objcopy -j .text -j .data -O ihex $INPUT $OUTPUT
