COMPILER=clang
CFLAGS=-emit-llvm -g 

all: parent rename_symbols_pass linked.bc

linked.bc: superglue.bc patched.bc vuln.bc can_dump.bc
	llvm-link -o $@ $^

patched.bc: parent
	PREFIX=patched opt -load-pass-plugin rename_symbols_pass/Build/libhw1.so -passes='hw1' ../patched/prog.bc -o patched.bc &> /dev/null

vuln.bc: parent
	PREFIX=vuln opt -load-pass-plugin rename_symbols_pass/Build/libhw1.so -passes='hw1' ../vuln/prog.bc -o vuln.bc &> /dev/null

SIZE?=1

.PHONY: superglue.bc
superglue.bc: superglue.c ../can_dump.h ../can_dump.c
	$(COMPILER) -c -Iinc $(CFLAGS) superglue.c ../can_dump.c -I/usr/include/openssl-1.0/ -I/usr/include/klee -DSIZE=$(SIZE)

parent:
	make -C ..

.PHONY: rename_symbols_pass
rename_symbols_pass:
	cd rename_symbols_pass; cmake -B Build
	make -C rename_symbols_pass/Build

clean:
	rm -f *.bc
	rm -rf klee-*
	make -C .. clean
	rm -rf rename_symbols_pass/Build
