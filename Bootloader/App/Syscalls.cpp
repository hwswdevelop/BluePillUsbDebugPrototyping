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
static void sysCallWriteUsbSerial(uint32_t arg0, uint32_t arg1, uint32_t arg2);

const SysCallFunc sysCallFuncTable[]={
	sysCallReset,
	sysCallEnableDisableIrq,
	sysCallMillis,
	sysCallWriteUsbSerial
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


#include "usb_common.h"
#include "Usb.h"

extern usbd_device *usbDevice;
extern bool usbInitialized;
volatile bool allowUsbTransfer = false;

static void sysCallWriteUsbSerial(uint32_t arg0, uint32_t arg1, uint32_t arg2){
	if (!usbInitialized) return;
	char* ptr = reinterpret_cast<char*>(arg0);
	size_t size = arg1;
	size_t offset = 0;

	if (usbDevice == nullptr) return;
	if  ( ((arg0 < 0x8004000) || (arg0 > 0x8020000)) &&
		  ((arg0 < 0x20001000) || (arg0 > 0x20005000))) return;
	if ((size == 0) || (size > 1024)) return;

	if  ( ((arg0 < 0x8004000) || ((arg0 + size) > 0x8020000)) &&
		  ((arg0 < 0x20001000) || ((arg0 + size) > 0x20005000))) return;

	if (!allowUsbTransfer) return;

	while(offset < size){
		size_t remain = size - offset;
		size_t count = (remain < BULK_EP_MAXPACKET) ? remain : BULK_EP_MAXPACKET;
		uint32_t allowCounter = 1000000;
		while( (!allowUsbTransfer) && (--allowCounter)){
			asm("nop");
		}
		if (!allowUsbTransfer) return;
		allowUsbTransfer = false;
		usbd_ep_write_packet(usbDevice, 0x82, reinterpret_cast<uint8_t*>(ptr + offset), static_cast<uint16_t>(count));
		offset += count;
	}

}

void usbSerialDataInCallback(usbd_device *usbd_dev, uint8_t ep){
	allowUsbTransfer = true;
}

