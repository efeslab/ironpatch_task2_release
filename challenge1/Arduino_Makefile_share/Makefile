AVR_GCC=$(HOME)/.arduino15/packages/arduino/tools/avr-gcc/4.8.1-arduino2/bin/avr-gcc
AVR_CXX=$(HOME)/.arduino15/packages/arduino/tools/avr-gcc/4.8.1-arduino2/bin/avr-g++
AVR_AR=$(HOME)/.arduino15/packages/arduino/tools/avr-gcc/4.8.1-arduino2/bin/avr-ar
BUILD_DIR=.
AVR_DIR=$(HOME)/.arduino15/packages/arduino/hardware/avr/1.6.2/
INCLUDE=-I$(AVR_DIR)/cores/arduino -I$(AVR_DIR)/variants/standard
GCC_FLAGS=-c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10813 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR
CXX_FLAGS=-c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10813 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR
CXX_FLAGS_LIBS=-c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -w -x c++ -E -CC -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10813 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR
LIB_PATH=$(HOME)/Arduino/libraries

all: linked_patched.bc linked_vuln.bc

##########################################
############ challenge code ##############
##########################################
BrakeFlasher_AVR_Vuln.ino.cpp: BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino
	arduino -v --pref build.path=Build_vuln --verify --board arduino:avr:uno BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino
	cp Build_vuln/sketch/BrakeFlasher_AVR_Vuln.ino.cpp .

BrakeFlasher_AVR_Vuln.E.cpp: BrakeFlasher_AVR_Vuln.ino.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) -I$(LIB_PATH)/CAN_BUS_Shield-master -I$(AVR_DIR)/libraries/SPI -I$(LIB_PATH)/elapsedMillis-master BrakeFlasher_AVR_Vuln.ino.cpp -E > BrakeFlasher_AVR_Vuln.E.cpp
	vim -s fixups.vim BrakeFlasher_AVR_Vuln.E.cpp

BrakeFlasher_AVR_Vuln.E.bc: BrakeFlasher_AVR_Vuln.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone BrakeFlasher_AVR_Vuln.E.cpp

###################
BrakeFlasher_AVR_Patched.ino.cpp: BrakeFlasher_AVR_Patched/BrakeFlasher_AVR_Patched.ino
	arduino -v --pref build.path=Build_patched --verify --board arduino:avr:uno BrakeFlasher_AVR_Patched/BrakeFlasher_AVR_Patched.ino
	cp Build_patched/sketch/BrakeFlasher_AVR_Patched.ino.cpp .

BrakeFlasher_AVR_Patched.E.cpp: BrakeFlasher_AVR_Patched.ino.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) -I$(LIB_PATH)/CAN_BUS_Shield-master -I$(AVR_DIR)/libraries/SPI -I$(LIB_PATH)/elapsedMillis-master BrakeFlasher_AVR_Patched.ino.cpp -E > BrakeFlasher_AVR_Patched.E.cpp
	vim -s fixups.vim BrakeFlasher_AVR_Patched.E.cpp

BrakeFlasher_AVR_Patched.E.bc: BrakeFlasher_AVR_Patched.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone BrakeFlasher_AVR_Patched.E.cpp

##########################################
############ core arduino CXX ############
##########################################
HardwareSerial.E.cpp: $(AVR_DIR)/cores/arduino/HardwareSerial.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) $(AVR_DIR)/cores/arduino/HardwareSerial.cpp -E > HardwareSerial.E.cpp
	vim -s fixups.vim HardwareSerial.E.cpp
	vim -s fix_HardwareSerial.vim HardwareSerial.E.cpp 
	vim -s fix_HardwareSerial2.vim HardwareSerial.E.cpp 

# -fms-extensions makes clang stop complaining about pointer sizes
# the information is from here: https://stackoverflow.com/questions/21232460/disabling-cast-from-pointer-to-smaller-type-uint32-t-error-in-clang/21233129
HardwareSerial.E.bc: HardwareSerial.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone -fms-extensions HardwareSerial.E.cpp

HardwareSerial0.E.cpp: $(AVR_DIR)/cores/arduino/HardwareSerial0.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) $(AVR_DIR)/cores/arduino/HardwareSerial0.cpp -E > HardwareSerial0.E.cpp
	vim -s fixups.vim HardwareSerial0.E.cpp

HardwareSerial0.E.bc: HardwareSerial0.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone -fms-extensions HardwareSerial0.E.cpp

Print.E.cpp: $(AVR_DIR)/cores/arduino/Print.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) $(AVR_DIR)/cores/arduino/Print.cpp -E > Print.E.cpp
	vim -s fixups.vim Print.E.cpp
	vim -s fixups_z.vim Print.E.cpp
	vim -s fix_Print.vim Print.E.cpp

Print.E.bc: Print.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone -fms-extensions Print.E.cpp

##########################################
############ core arduino C ##############
##########################################
wiring_digital.E.c: $(AVR_DIR)/cores/arduino/wiring_digital.c
	$(AVR_GCC) $(GCC_FLAGS) $(INCLUDE) $(AVR_DIR)/cores/arduino/wiring_digital.c -E > wiring_digital.E.c
	vim -s fixups_z2.vim wiring_digital.E.c
	vim -s fixups_z2.vim wiring_digital.E.c
	vim -s delete_z2.vim wiring_digital.E.c
	vim -s delete_z2.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c
	vim -s fixups_z.vim wiring_digital.E.c

wiring_digital.E.bc: wiring_digital.E.c
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone -fms-extensions wiring_digital.E.c

##########################################
############ libraries ###################
##########################################
mcp_can.E.cpp: $(LIB_PATH)/CAN_BUS_Shield-master/mcp_can.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) -I$(LIB_PATH)/CAN_BUS_Shield-master -I$(AVR_DIR)/libraries/SPI -I$(LIB_PATH)/elapsedMillis-master $(LIB_PATH)/CAN_BUS_Shield-master/mcp_can.cpp -E > mcp_can.E.cpp
	vim -s fixups.vim mcp_can.E.cpp
	vim -s fix_mcp_can.vim mcp_can.E.cpp 
	vim -s fix_mcp_can2.vim mcp_can.E.cpp 
	vim -s fix_mcp_can3.vim mcp_can.E.cpp 
	vim -s fix_mcp_can4.vim mcp_can.E.cpp 
	vim -s fix_mcp_can5.vim mcp_can.E.cpp 

mcp_can.E.bc: mcp_can.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone mcp_can.E.cpp

SPI.E.cpp: $(AVR_DIR)/libraries/SPI/SPI.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) -I$(LIB_PATH)/CAN_BUS_Shield-master -I$(AVR_DIR)/libraries/SPI -I$(LIB_PATH)/elapsedMillis-master $(AVR_DIR)/libraries/SPI/SPI.cpp -E > SPI.E.cpp
	vim -s fixups.vim SPI.E.cpp
	vim -s fixups_z.vim SPI.E.cpp
	vim -s fixups_z.vim SPI.E.cpp
	vim -s fixups_z2.vim SPI.E.cpp

SPI.E.bc: SPI.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone -fms-extensions SPI.E.cpp

##########################################
############ link ########################
##########################################
glue.E.cpp: glue.cpp
	$(AVR_CXX) $(CXX_FLAGS) $(INCLUDE) -I$(LIB_PATH)/CAN_BUS_Shield-master -I$(AVR_DIR)/libraries/SPI -I$(LIB_PATH)/elapsedMillis-master -I /usr/include/klee glue.cpp -E > glue.E.cpp
	vim glue.E.cpp -s fixups.vim

glue.E.bc: glue.E.cpp
	clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone glue.E.cpp

linked_vuln.bc: glue.E.bc BrakeFlasher_AVR_Vuln.E.bc HardwareSerial.E.bc HardwareSerial0.E.bc Print.E.bc mcp_can.E.bc SPI.E.bc wiring_digital.E.bc
	llvm-link glue.E.bc BrakeFlasher_AVR_Vuln.E.bc HardwareSerial.E.bc HardwareSerial0.E.bc Print.E.bc mcp_can.E.bc SPI.E.bc wiring_digital.E.bc -o linked_vuln.bc 

linked_patched.bc: glue.E.bc BrakeFlasher_AVR_Patched.E.bc HardwareSerial.E.bc HardwareSerial0.E.bc Print.E.bc mcp_can.E.bc SPI.E.bc wiring_digital.E.bc
	llvm-link glue.E.bc BrakeFlasher_AVR_Patched.E.bc HardwareSerial.E.bc HardwareSerial0.E.bc Print.E.bc mcp_can.E.bc SPI.E.bc wiring_digital.E.bc -o linked_patched.bc 

##########################################
############ clean #######################
##########################################
clean:
	rm -f BrakeFlasher_AVR_Vuln.ino.d
	rm -f BrakeFlasher_AVR_Patched.ino.d
	rm -f BrakeFlasher_AVR_Vuln.E.cpp
	rm -f BrakeFlasher_AVR_Vuln.E.bc
	rm -f HardwareSerial.d
	rm -f HardwareSerial.E.cpp
	rm -f HardwareSerial.E.bc
	rm -f HardwareSerial0.d
	rm -f HardwareSerial0.E.cpp
	rm -f HardwareSerial0.E.bc
	rm -f Print.d
	rm -f Print.E.cpp
	rm -f Print.E.bc
	rm -f glue.d
	rm -f glue.E.cpp
	rm -f glue.E.bc
	rm -f mcp_can.d
	rm -f mcp_can.E.cpp
	rm -f mcp_can.E.bc
	rm -f SPI.d
	rm -f SPI.E.cpp
	rm -f SPI.E.bc
	rm -f wiring_digital.d
	rm -f wiring_digital.E.c
	rm -f wiring_digital.E.bc
	rm -f linked_vuln.bc
	rm -f linked_patched.bc
	rm -f BrakeFlasher_AVR_Vuln.E.bc
	rm -f BrakeFlasher_AVR_Vuln.E.cpp
	rm -f BrakeFlasher_AVR_Patched.E.bc
	rm -f BrakeFlasher_AVR_Patched.E.cpp
	rm -rf Build_vuln
	rm -rf Build_patched
	rm -f BrakeFlasher_AVR_Vuln.ino.cpp
	rm -f BrakeFlasher_AVR_Patched.ino.cpp
