#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "Usage: $0 <file_name.ino.hex>"
fi

INPUT=$1

if ! [[ $INPUT =~ \.ino\.hex$ ]];
then
	echo "Bad extension"
	echo "Usage: $0 <file_name.ino.hex>"
	exit 1
fi

echo "$0: input: $INPUT"

avrdude -v -p atmega328p -carduino -P/dev/ttyACM0 -b115200 -D -Uflash:w:$INPUT:i
