This is a directory for compiling challange2 code to LLVM IR.

I am working on putting my llvm Makefiles in
[Nucleo-32/Makefile](Nucleo-32/Makefile),
[Nucleo-32-patched/Makefile](Nucleo-32-patched/Makefile).

Note that these are **not** the original Makefiles.
The original Makefiles are at
[Nucleo-32/Debug/makefile](Nucleo-32/Debug/makefile),
[Nucleo-32-patched/Debug/makefile](Nucleo-32-patched/Debug/makefile).

Also, the code we received from AMP contains lots of `.o` and other intermediate files. I deleted them here.

For "native" compilation, the 
[build\_script.sh](build_script.sh)
has to be executed from within 
[Nucleo-32/Debug](Nucleo-32/Debug)
or
[Nucleo-32-patched/Debug](Nucleo-32-patched/Debug).
