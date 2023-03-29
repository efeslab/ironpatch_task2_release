#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "Usage: $0 <file_name.ino>"
fi

INPUT=$1

if ! [[ $INPUT =~ \.ino$ ]];
then
	echo "Bad extension"
	echo "Usage: $0 <file_name.ino>"
	exit 1
fi

echo "$0: input: $INPUT"
echo "$0: Put the output in a directory called Build"

arduino -v --pref build.path=Build --verify --board arduino:avr:uno $INPUT
