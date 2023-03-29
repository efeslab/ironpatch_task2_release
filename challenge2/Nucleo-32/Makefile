export PATH := ${PATH}:/opt/stm32cubeide/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.linux64_1.4.0.202007081208/tools/bin:/opt/stm32cubeide/plugins/com.st.stm32cube.ide.mcu.externaltools.make.linux64_1.4.0.202007081208/tools/bin:/home/mini/.local/bin:/home/mini/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/bin:/opt/android-ndk:/opt/android-sdk/platform-tools:/opt/android-sdk/tools:/opt/android-sdk/tools/bin:/usr/lib/jvm/default/bin:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl:/opt/android-ndk:/opt/android-sdk/platform-tools:/opt/android-sdk/tools:/opt/android-sdk/tools/bin:/usr/lib/jvm/default/bin

compiler_prefix:=/opt/stm32cubeide/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.linux64_1.4.0.202007081208/tools/bin/arm-none-eabi-
CC_FLAGS=-mcpu=cortex-m0 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F042x6 -DDEBUG -c
CC_FLAGS2=-O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP
FLAGS3=--specs=nano.specs -mfloat-abi=soft -mthumb
INCLUDE=-IDrivers/STM32F0xx_HAL_Driver/Inc -IDrivers/CMSIS/Include -ICore/Inc -IDrivers/STM32F0xx_HAL_Driver/Inc/Legacy -IDrivers/CMSIS/Device/ST/STM32F0xx/Include

# Debug/Drivers/STM32F0xx_HAL_Driver/Src/*.o 
driver_objects= \
	stm32f0xx_hal.o \
	stm32f0xx_hal_can.o \
	stm32f0xx_hal_cortex.o \
	stm32f0xx_hal_dma.o \
	stm32f0xx_hal_exti.o \
	stm32f0xx_hal_flash.o \
	stm32f0xx_hal_flash_ex.o \
	stm32f0xx_hal_gpio.o \
	stm32f0xx_hal_i2c.o \
	stm32f0xx_hal_i2c_ex.o \
	stm32f0xx_hal_pwr.o \
	stm32f0xx_hal_pwr_ex.o \
	stm32f0xx_hal_rcc.o \
	stm32f0xx_hal_rcc_ex.o \
	stm32f0xx_hal_tim.o \
	stm32f0xx_hal_tim_ex.o \
	stm32f0xx_hal_uart.o \
	stm32f0xx_hal_uart_ex.o 

# Debug/Core/Src/*.o
cpp_objects= \
	Bumper.o \
	UnitTesting.o \
	main.o

# Debug/Core/Src/*.o
core_objects = \
	stm32f0xx_hal_msp.o \
	stm32f0xx_it.o \
	syscalls.o \
	sysmem.o \
	system_stm32f0xx.o

all: linked.bc

run: linked.bc
	lli linked.bc -force-interpreter=true

linked.bc: $(patsubst %.o,Debug/Drivers/STM32F0xx_HAL_Driver/Src/%.o,$(driver_objects)) $(patsubst %.o,Debug/Core/Src/%.o,$(cpp_objects)) $(patsubst %.o,Debug/Core/Src/%.o,$(core_objects))
	llvm-link -o linked.bc $(patsubst %.o,%.bc,$(driver_objects)) $(patsubst %.o,%.bc,$(cpp_objects)) $(patsubst %.o,%.bc,$(core_objects))

Debug/Drivers/STM32F0xx_HAL_Driver/Src/%.o: Drivers/STM32F0xx_HAL_Driver/Src/%.c
	$(compiler_prefix)gcc "$<" $(CC_FLAGS) $(INCLUDE) $(CC_FLAGS2) -MF"Debug/Drivers/STM32F0xx_HAL_Driver/Src/$*.d" -MT"$@" $(FLAGS3) -o "$@"
	clang -emit-llvm -c "$<" -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F042x6 -DDEBUG -c $(INCLUDE) -O0 -ffunction-sections -fdata-sections -Wall -MMD -MP -MF"Debug/Drivers/STM32F0xx_HAL_Driver/Src/$*.d" -MT"$@"

Debug/Core/Src/%.o: Core/Src/%.cpp
	$(compiler_prefix)g++ "$<" -mcpu=cortex-m0 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DSTM32F042x6 -DDEBUG -c $(INCLUDE) -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Debug/Core/Src/$*.d" -MT"$@" $(FLAGS3) -o "$@"
	clang -emit-llvm -c "$<" -mcpu=cortex-m0 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DSTM32F042x6 -DDEBUG -c $(INCLUDE) -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -MMD -MP -MF"Debug/Core/Src/$*.d" -MT"$@" $(FLAGS3)

Debug/Core/Src/%.o: Core/Src/%.c
	$(compiler_prefix)gcc "$<" $(CC_FLAGS) $(INCLUDE) $(CC_FLAGS2) -MF"Debug/Core/Src/$*.d" -MT"$@" $(FLAGS3) -o "$@"
	 clang -emit-llvm -c "$<" $(CC_FLAGS) $(INCLUDE) -O0 -ffunction-sections -fdata-sections -Wall -MMD -MP -MF"Debug/Core/Src/$*.d" -MT"$@" $(FLAGS3)

clean:
	rm -f $(patsubst %.o,Debug/Drivers/STM32F0xx_HAL_Driver/Src/%.o,$(driver_objects))
	rm -f $(patsubst %.o,Debug/Drivers/STM32F0xx_HAL_Driver/Src/%.d,$(driver_objects))
	rm -f $(patsubst %.o,Debug/Core/Src/%.o,$(cpp_objects))
	rm -f $(patsubst %.o,Debug/Core/Src/%.d,$(cpp_objects))
	rm -f $(patsubst %.o,Debug/Core/Src/%.o,$(core_objects))
	rm -f $(patsubst %.o,Debug/Core/Src/%.d,$(core_objects))
	rm -f $(patsubst %.o,%.bc,$(driver_objects))
	rm -f $(patsubst %.o,%.bc,$(cpp_objects))
	rm -f $(patsubst %.o,%.bc,$(core_objects))
	rm -f linked.bc
	#rm -f $(Debug/Core/Src/:core_objects:.o=.d)
