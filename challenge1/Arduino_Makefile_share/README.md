### Overview
The code in the repo contains the `.ino` source code of the challenge problems in their containing directories (requirement of the build system). The Makefile takes these `.ino` files and runs the Arduino command line build system that compiles the code into binary that can run on Arduino. This is how we generate `.ino.cpp` files, which are basically `.ino` files with `#include<Arduino.h>` at the top. From there, the Makefile uses the `avr-gcc` compiler with the flag `-E` to compile the challenge code and all the needed libraries. Then the Makefile runs a series of `.vim` scripts to help emulate the library functions. Lastly, the Makefile runs `clang` to generate LLVM bytecode and `llvm-link` to link everything together

### Prerequesits
Before running `make` install the Arduino IDE. I am using Archlinux where the version of `arduino` is `1.8.13`. Note that the default Ubuntu `arduino` version is `1.0.5`, which does not support the CLI mode.
I also installed the required libraries using the GUI.
Our team was able to install the correct Arduino version on Ubuntu, but I did not run this makefile on Ubuntu.

Outputs of the Makefile are `linked_patched.bc` `linked_vuln.bc`.

### Useful hack
A useful hack for inspecting the native build process is running the `arduino` command with the `-v` flag, since it outputs most of the build steps (see usage in the makefile).
It outputs the installation path of all the tools in your system.

### FIles in this directory:
- Challange code
  - `BrakeFlasher_AVR_Patched`
  - `BrakeFlasher_AVR_Vuln`
- Vim fix scripts. They do things like deleting inline assembly ad add hooks to call `glue.cpp` where needed
  - `delete_z2.vim`
  - `fix_HardwareSerial2.vim`
  - `fix_HardwareSerial.vim`
  - `fix_mcp_can2.vim`
  - `fix_mcp_can3.vim`
  - `fix_mcp_can4.vim`
  - `fix_mcp_can5.vim`
  - `fix_mcp_can.vim`
  - `fix_Print.vim`
  - `fixups.vim`
  - `fixups_z2.vim`
  - `fixups_z.vim`
- file that contains `main()`
  - `glue.cpp`
- Makefile
  - `Makefile`
- This readme
  - `README.md`
