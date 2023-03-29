I embedded here the slicer from here <https://github.com/mchalupa/dg>, commit `4c69260936aeb35eca05fe069301c39ce4ec2dd7`
To compile, run [build_dg.sh](build_dg.sh)

## Before starting
- In [code.c](code.c) remove (if was not removed):
  - `#include "klee.h"`
  - `klee_make_symbolic(input, sizeof(input), "input");`

## No slicer version
- Add:
  - `#include "klee.h"`
  - `klee_make_symbolic(input, sizeof(input), "input");`
- compile (these are equivalent)
  - `make build`
  - `clang -c -emit-llvm code.c -o code.bc -fno-discard-value-names -I/usr/include/klee`
- Run with klee
  - `klee code.bc`

## With slicer
- Remove:
  - `#include "klee.h"`
  - `klee_make_symbolic(input, sizeof(input), "input");`
- save
- `make build`
- Slice the code based on the slicing criteria:
  - `make slice`
  - `./dg/Build/tools/llvm-slicer -annotate slice -c __assert_fail code.bc`
- Inspect the annotated slice file:
  - `make inspect`
  - `vi code-debug.ll`
- Add the klee function to the llvm code
  - `llvm-dis code.sliced`
  - `vi code.sliced.ll`
    - In the beginning of the file `@.str_name = private unnamed_addr constant [6 x i8] c"input\00", align 1`
    - In the beginning of main, after `call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %0, i8* align 1 getelementptr inbounds ([8 x i8], [8 x i8]* @__const.main.input, i32 0, i32 0), i64 8, i1 false)` add:
      - `%arraydecay = getelementptr inbounds [8 x i8], [8 x i8]* %input, i64 0, i64 0`
      - `call void @klee_make_symbolic(i8* %arraydecay, i64 8, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str_name, i64 0, i64 0))`
   - After the main function add:
     - `declare void @klee_make_symbolic(i8*, i64, i8*) #2`
   - `llvm-as code.sliced.ll`
- Run with klee:
  - `make klee`
  - `klee code.sliced.bc`

