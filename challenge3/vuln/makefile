CFLAGS=-emit-llvm -g
COMPILER=clang

all: vuln

vuln: main

main: main.bc gpio.bc bumper.bc glue.bc
	llvm-link main.bc gpio.bc bumper.bc glue.bc -o linked.bc

main.bc: src/main.c
	$(COMPILER) -c -Iinc $(CFLAGS) src/main.c

gpio.bc: src/gpio.c
	$(COMPILER) -c -Iinc $(CFLAGS) src/gpio.c

bumper.bc: src/bumper.c
	$(COMPILER) -c -Iinc $(CFLAGS) src/bumper.c

glue.bc: glue.c
	$(COMPILER) -c -Iinc $(CFLAGS) glue.c -I/usr/include/klee

clean:
	rm -f *.bc

.PHONY: all vuln patch clean
