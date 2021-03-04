/*
 * Syscalls.cpp
 *
 *  Created on: 1 мар. 2021 г.
 *      Author: Evgeny
 */
#include <stdint.h>
#include <stddef.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f1/st_usbfs.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/mpu.h>
#include "Debug.h"

typedef void (*SysCallFunc)(uint32_t arg0, uint32_t arg1, uint32_t arg2);

static void sysCallMillis(uint32_t arg0, uint32_t arg1, uint32_t arg2);
static void sysCallReset(uint32_t arg0, uint32_t arg1, uint32_t arg2);
static void sysCallEnableDisableIrq(uint32_t arg0, uint32_t arg1, uint32_t arg2);

const SysCallFunc sysCallFuncTable[]={
	sysCallReset,
	sysCallEnableDisableIrq,
	sysCallMillis,
};

static constexpr const size_t SysCallFuncCount = sizeof(sysCallFuncTable) / sizeof(SysCallFunc);

extern "C" void SVC_Handler(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t funcNo){
	if (funcNo < SysCallFuncCount){
		sysCallFuncTable[funcNo](arg0, arg1, arg2);
	}
}

extern volatile uint32_t timeFromSystemStartedMs;

static void sysCallReset(uint32_t arg0, uint32_t arg1, uint32_t arg2){
	debugStartFirmware();
	debugResume(false);
}

static void sysCallMillis(uint32_t arg0, uint32_t arg1, uint32_t arg2){
	if (arg0 & 0x03) return;
	if (arg0 < 0x20001000) return;
	if (arg0 > 0x20005000) return;
	*((volatile uint32_t*)arg0) = timeFromSystemStartedMs;
}


static void sysCallEnableDisableIrq(uint32_t arg0, uint32_t arg1, uint32_t arg2){
	if (arg0 > 255) return;
	if (arg1 != 0){
		setDebugCode(9);
		nvic_enable_irq(arg0);
	} else {
		setDebugCode(10);
		nvic_disable_irq(arg0);
	}
	setDebugCode(11);
}

