CFLAGS=-s
COMPILER=gcc

all: vuln patch

vuln: 
	if [ ! -d ../build ]; then mkdir ../build; fi
	./patch_unpatch_script vuln
	$(MAKE) main TYPE=

patch: 
	if [ ! -d ../build ]; then mkdir ../build; fi
	./patch_unpatch_script patch
	$(MAKE) main TYPE=_patched

main: main.o gpio.o bumper.o
	$(COMPILER) $(CFLAGS) -o ../build/program_c${TYPE} main.o gpio.o bumper.o

main.o: src/main.c
	$(COMPILER) -c -Iinc $(CFLAGS) src/main.c

gpio.o: src/gpio.c
	$(COMPILER) -c -Iinc $(CFLAGS) src/gpio.c

bumper.o: src/bumper.c
	$(COMPILER) -c -Iinc $(CFLAGS) src/bumper.c

clean:
	rm -f *.o

.PHONY: all vuln patch clean
