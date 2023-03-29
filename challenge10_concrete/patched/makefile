#CFLAGS=-static -O2 -Wall
CFLAGS=-emit-llvm -g
CLINK=-s
#COMPILER=gcc
COMPILER=clang

all: prog.bc

prog.bc: main.bc gpio.bc bumper.bc logging.bc EVP_aes_256_cbc.bc transport.bc watchdog.bc spn.bc monitor.bc glue.bc
	llvm-link -o prog.bc main.bc transport.bc gpio.bc bumper.bc logging.bc EVP_aes_256_cbc.bc watchdog.bc spn.bc monitor.bc glue.bc

glue.bc: glue.c
	$(COMPILER) -c -Iinc $(CFLAGS) glue.c -I/home/musah/klee/klee/include/klee

patch:
	#$(MAKE) program_c

monitor.bc: src/monitor.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/monitor.c

spn.bc: src/spn.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/spn.c

watchdog.bc: src/watchdog.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/watchdog.c

transport.bc: src/transport.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/transport.c

util.bc: src/util.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/util.c

logging.bc: src/logging.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/logging.c

main.bc: src/main.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/main.c

gpio.bc: src/gpio.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/gpio.c

bumper.bc: src/bumper.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/bumper.c

EVP_aes_256_cbc.bc: src/EVP_aes_256_cbc.c
	$(COMPILER) -Wall -c -Iinc $(CFLAGS) src/EVP_aes_256_cbc.c

libs:
	$(COMPILER) -fPIC -Iinc -shared -lcrypto  -o ../build/libaes.so src/EVP_aes_256_cbc.c

clean:
	rm -f *.bc
	rm -rf klee-*

.PHONY: all program_c patch unpatch clean
