CFLAGS=-emit-llvm -g
COMPILER=clang

all: vuln patched superglue.bc
	llvm-link vuln.bc patched.bc superglue.bc -o linked.bc

.PHONY: vuln
vuln:
	make -C vuln
	PREFIX=vuln opt -load-pass-plugin ../../simplified_challange1/rename_symbols_pass/Build/libhw1.so -passes='hw1' vuln/linked.bc -o vuln.bc &> /dev/null

.PHONY: patched
patched:
	make -C patched
	PREFIX=patched opt -load-pass-plugin ../../simplified_challange1/rename_symbols_pass/Build/libhw1.so -passes='hw1' patched/linked.bc -o patched.bc &> /dev/null

superglue.bc: superglue.c
	$(COMPILER) -c -Iinc $(CFLAGS) superglue.c -I/usr/include/klee

clean:
	make clean -C vuln
	make clean -C patched
	rm -f vuln.bc
	rm -f patched.bc
	rm -f linked.bc
	rm -f superglue.bc
