### to compile and run with KLEE:
First, compile the sketch using `./compile.sh BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino`. Make sure to add the `-v` flag to `compile.sh`.

Find this in the output:
```
Compiling sketch...
/home/mini/.arduino15/packages/arduino/tools/avr-gcc/4.8.1-arduino2/bin/avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10813 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR -I/home/mini/.arduino15/packages/arduino/hardware/avr/1.6.2/cores/arduino -I/home/mini/.arduino15/packages/arduino/hardware/avr/1.6.2/variants/standard -I/home/mini/Arduino/libraries/CAN_BUS_Shield-master -I/home/mini/.arduino15/packages/arduino/hardware/avr/1.6.2/libraries/SPI -I/home/mini/Arduino/libraries/elapsedMillis-master /home/mini/workspace/ironpatch/Arduino_klee/Build/sketch/BrakeFlasher_AVR_Vuln.ino.cpp -o /home/mini/workspace/ironpatch/Arduino_klee/Build/sketch/BrakeFlasher_AVR_Vuln.ino.cpp.o
```

The interesting files are now in `Build/sketch`. `cd` there.
The command above works in this directory. You can delete the .o file and re-generate it.

replace `-o` with `-E`:
```
/home/mini/.arduino15/packages/arduino/tools/avr-gcc/4.8.1-arduino2/bin/avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10813 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR -I/home/mini/.arduino15/packages/arduino/hardware/avr/1.6.2/cores/arduino -I/home/mini/.arduino15/packages/arduino/hardware/avr/1.6.2/variants/standard -I/home/mini/Arduino/libraries/CAN_BUS_Shield-master -I/home/mini/.arduino15/packages/arduino/hardware/avr/1.6.2/libraries/SPI -I/home/mini/Arduino/libraries/elapsedMillis-master /home/mini/workspace/ironpatch/Arduino_klee/Build/sketch/BrakeFlasher_AVR_Vuln.ino.cpp -E > BrakeFlasher_AVR_Vuln.E.cpp
```
add a main func to the end of the file you just generates:
```
int main() {
	setup();
	while(1) {
		loop();
	}
}
```

In the same file, delete inline assembly that contains `=w`

Generate LLVM IR code:
```
clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone BrakeFlasher_AVR_Vuln.E.cpp
```


Run with klee :) `klee BrakeFlasher_AVR_Vuln.E.bc`

Now figure out what to do with all the errors in the output :(
```
KLEE: output directory is "/home/mini/workspace/ironpatch/Arduino_klee/Build/sketch/klee-out-0"
KLEE: Using Z3 solver backend
KLEE: WARNING: undefined reference to variable: Serial
KLEE: WARNING: undefined reference to function: _ZN14HardwareSerial5beginEmh
KLEE: WARNING: undefined reference to function: _ZN5Print5printEPKc
KLEE: WARNING: undefined reference to function: _ZN5Print5printEhi
KLEE: WARNING: undefined reference to function: _ZN5Print5printEii
KLEE: WARNING: undefined reference to function: _ZN5Print5printEmi
KLEE: WARNING: undefined reference to function: _ZN5Print7printlnEPKc
KLEE: WARNING: undefined reference to function: _ZN5Print7printlnEji
KLEE: WARNING: undefined reference to function: _ZN7MCP_CAN12checkReceiveEv
KLEE: WARNING: undefined reference to function: _ZN7MCP_CAN12readMsgBufIDEPmPhS1_
KLEE: WARNING: undefined reference to function: _ZN7MCP_CAN5beginEhh
KLEE: WARNING: undefined reference to function: _ZN7MCP_CAN9init_FiltEhhm
KLEE: WARNING: undefined reference to function: _ZN7MCP_CAN9init_MaskEhhm
KLEE: WARNING: undefined reference to function: _ZN7MCP_CANC1Eh
KLEE: WARNING: undefined reference to function: analogWrite
KLEE: WARNING: undefined reference to function: delay
KLEE: WARNING: undefined reference to function: init
KLEE: WARNING: undefined reference to function: micros
KLEE: WARNING: undefined reference to function: millis
KLEE: WARNING: undefined reference to function: pinMode
KLEE: ERROR: unable to load symbol(Serial) while initializing globals.
```

Update: I removed all the function calls that caused wrrors, and the declaration of `Serial` as being extern, and managed to get it tu run.
