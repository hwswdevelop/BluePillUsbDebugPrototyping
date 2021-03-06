################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../System/Core/gpio.cpp 

OBJS += \
./System/Core/gpio.o 

CPP_DEPS += \
./System/Core/gpio.d 


# Each subdirectory must supply rules for building sources it contributes
System/Core/gpio.o: ../System/Core/gpio.cpp System/Core/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m3 -std=gnu++17 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -I"D:/BluePuillDebugOverUsb/Template/System" -I"D:/BluePuillDebugOverUsb/Template/System/Core" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"System/Core/gpio.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

