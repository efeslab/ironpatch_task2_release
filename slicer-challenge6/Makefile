.PHONY: build
build:
	clang -c -emit-llvm code.c -o code.bc -fno-discard-value-names -I/usr/include/klee

.PHONY: slice
slice:
	./dg/Build/tools/llvm-slicer -annotate slice -c __assert_fail code.bc

.PHONY: inspect
inspect:
	vi code-debug.ll

.PHONY: klee
klee:
	klee code.sliced.bc
