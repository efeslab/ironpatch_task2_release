
startloc=../../../
munit_dir=$(startloc)/munit
munit_file=$(munit_dir)/munit
srcfiles=main.c bumper.c gpio.c logging.c transport.c

# Which testing to be done
ENC?=aes
ifeq ($(ENC),aes) 
srcfiles += EVP_aes_256_cbc.c
desaes=2
else
srcfiles +=  EVP_des_ede3_cbc.c
desaes=1
endif

UP?=unpatched
ifeq ($(UP),patched) 
up=1
else
up=2
endif

srcdir=$(shell find $(startloc) -type f -name $(word 2,$(srcfiles)) -exec dirname "{}" \;)
includedir=$(subst src,inc,$(srcdir))

CC=gcc
TESTENVIRONMENT:=
CFLAGS:=-I $(munit_dir) -I $(includedir) -DUP=$(up) -DENC=$(desaes) -g -lcrypto 
SRC:=$(addprefix $(srcdir)/,$(srcfiles))
OBJS:=$(patsubst %.c,%.o,$(SRC))

all: run # runpatch # No patch for Challenge 07

tests: $(munit_file).c $(OBJS) tests.c
	echo "Called regular"
	$(shell objcopy -N main $(srcdir)/main.o $(srcdir)/main.o)
	$(CC) $(CFLAGS) $^ -o $@

testspatch: $(patsubst %main.o,,$(OBJS))
	echo "Called testspatch"
	patch $(includedir)/logging.h $(includedir)/logging.patch
	patch $(srcdir)/logging.c $(srcdir)/logging.patch
	patch $(srcdir)/EVP_aes_256_cbc.c $(srcdir)/EVP_aes_256_cbc.patch
	touch $(srcdir)/main.c
	$(MAKE) -e UP=patched tests
	patch -R $(includedir)/logging.h $(includedir)/logging.patch
	patch -R $(srcdir)/logging.c $(srcdir)/logging.patch
	patch -R $(srcdir)/EVP_aes_256_cbc.c $(srcdir)/EVP_aes_256_cbc.patch

# Testing for this script
me:
	@echo PWD: $(PWD)
	@echo startloc: $(startloc)
	@echo SRC: $(SRC)
	@echo includedir: $(includedir)

clean:
	rm $(OBJS) tests
	
run: tests
	# $(TESTENVIRONMENT) ./tests # --show-stderr --log-visible debug
	$(TESTENVIRONMENT) ./tests

runpatch: testspatch
	$(TESTENVIRONMENT) ./tests

.PHONY: all run me clean testspatch runpatch
