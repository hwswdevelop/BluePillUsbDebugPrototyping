################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../System/svccalls.s 

C_SRCS += \
../System/syscalls.c \
../System/sysmem.c 

CPP_SRCS += \
../System/delay.cpp \
../System/mainInternal.cpp 

S_DEPS += \
./System/svccalls.d 

C_DEPS += \
./System/syscalls.d \
./System/sysmem.d 

OBJS += \
./System/delay.o \
./System/mainInternal.o \
./System/svccalls.o \
./System/syscalls.o \
./System/sysmem.o 

CPP_DEPS += \
./System/delay.d \
./System/mainInternal.d 


# Each subdirectory must supply rules for building sources it contributes
System/delay.o: ../System/delay.cpp System/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++17 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -I"D:/BluePuillDebugOverUsb/Template/System" -I"D:/BluePuillDebugOverUsb/Template/System/Core" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"System/delay.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
System/mainInternal.o: ../System/mainInternal.cpp System/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++17 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -I"D:/BluePuillDebugOverUsb/Template/System" -I"D:/BluePuillDebugOverUsb/Template/System/Core" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"System/mainInternal.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
System/svccalls.o: ../System/svccalls.s System/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m3 -g3 -c -x assembler-with-cpp -MMD -MP -MF"System/svccalls.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"
System/syscalls.o: ../System/syscalls.c System/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -I"D:/BluePuillDebugOverUsb/Template/System" -I"D:/BluePuillDebugOverUsb/Template/System/Core" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"System/syscalls.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
System/sysmem.o: ../System/sysmem.c System/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -I"D:/BluePuillDebugOverUsb/Template/System" -I"D:/BluePuillDebugOverUsb/Template/System/Core" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"System/sysmem.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

