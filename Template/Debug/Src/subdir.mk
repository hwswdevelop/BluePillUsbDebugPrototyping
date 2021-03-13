################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/main.cpp 

OBJS += \
./Src/main.o 

CPP_DEPS += \
./Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
Src/main.o: ../Src/main.cpp Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++17 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -I"D:/BluePillUsbDebugPrototyping/Template/System" -I"D:/BluePillUsbDebugPrototyping/Template/System/Core" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Src/main.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

