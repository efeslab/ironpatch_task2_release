#!/bin/bash

for i in `seq 10`; do
	make SIZE=$i && klee linked.bc
done
