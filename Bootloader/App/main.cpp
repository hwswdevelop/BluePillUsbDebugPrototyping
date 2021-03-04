/*
 * main.cpp
 *
 *  Created on: 4 февр. 2021 г.
 *      Author: Evgeny
 */

#include "Debug.h"
#include "usb_common.h"
#include "Usb.h"
#include "DbgCommands.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f1/st_usbfs.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/mpu.h>
#include "Memory.h"

uint8_t usbCmdFrame[64];
uint8_t usbAnsFrame[64];

void usbDataInCallback(usbd_device *usbd_dev, uint8_t ep){
	asm("nop");
}

extern "C" void systemMain();

constexpr inline size_t getValidatedAlign(size_t align){
	const bool alignIsInvalid = (align - 1) & align;
	const size_t validatedAlign = (alignIsInvalid) ? 1 : align;
	return validatedAlign;
}

template<typename HeadType>
constexpr inline const size_t payloadSize(const size_t maxBufferSize, const size_t requiredDataSize, const size_t align){
	if ( sizeof(HeadType) > maxBufferSize ) return 0;
	const size_t maxPayloadSize = maxBufferSize - sizeof(HeadType);
	const size_t minAlignSize = getValidatedAlign(align);
	const size_t payloadSize = (maxPayloadSize < requiredDataSize) ? maxPayloadSize : requiredDataSize;
	const size_t alignedPayloadSize = payloadSize & ( ~(minAlignSize - 1) );
	return alignedPayloadSize;
}

template<typename HeadType>
constexpr inline bool dbgHeadValid( const uint8_t* const buf, const size_t size, const DebugCommand cmd ){
	const size_t headSize = sizeof(HeadType);
	if (headSize > size) return false;
	const HeadType* head = reinterpret_cast<const HeadType*>(buf);
	return ( static_cast<DebugCommand>(head->cmd) == cmd );
}

bool isFlashRegion(uint32_t address){
	bool isFlash = ( ( address >= 0x08004000 ) &&
			 	     ( address < 0x08020000 ) );
	return isFlash;
}

bool flashWrite(const uint32_t address, const uint8_t* const data, const size_t size){
	flash_unlock();
	for(size_t dataIndex = 0; dataIndex < size; dataIndex += sizeof(uint16_t)){
		uint16_t value = data[dataIndex];
		value |= ( ((uint16_t)data[dataIndex + 1]) << 8 );
		flash_program_half_word( address + dataIndex, value );
	}
	return true;
}

size_t memoryReadCommand( const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize ){
	if ( !dbgHeadValid<MemReadWriteCmdHead>( inBuf, inSize, DebugCommand::MemoryRead) ) return 0;
	const MemReadWriteCmdHead* const cmdHead = reinterpret_cast<const MemReadWriteCmdHead* const>(inBuf);
	const uint32_t address = cmdHead->address;
	const uint32_t requiredReadSize = cmdHead->byteCount;
	const uint32_t align = cmdHead->align;
	const size_t bytesToRead = payloadSize<MemReadWriteAnsHead>(outSize, requiredReadSize, align);
	MemReadWriteAnsHead* const ansHead = reinterpret_cast<MemReadWriteAnsHead* const>(outBuf);
	ansHead->cmd = static_cast<uint8_t>(DebugCommand::MemoryRead);
	ansHead->align = align;
	ansHead->address = address;
	ansHead->byteCount = bytesToRead;
	bool accessible = (checkMemRegionAccess(cmdHead->address, cmdHead->address + bytesToRead) & 0x01);
	if (accessible){
		dbg_memcpy( outBuf + sizeof(MemReadWriteAnsHead), reinterpret_cast<uint8_t*>(address), bytesToRead, align );
	} else {
		for(uint32_t index = 0; index < ansHead->byteCount; index++){
			*(outBuf + sizeof(struct MemReadWriteAnsHead) + index) = 0xFF;
		}
	}
	return sizeof(MemReadWriteAnsHead) + bytesToRead;
}

size_t memoryWriteCommand(const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize){
	if ( !dbgHeadValid<MemReadWriteCmdHead>( inBuf, inSize, DebugCommand::MemoryWrite) ) return 0;
	const MemReadWriteCmdHead* const cmdHead = reinterpret_cast<const MemReadWriteCmdHead* const>(inBuf);
	const uint32_t address = cmdHead->address;
	const uint32_t requiredWriteSize = cmdHead->byteCount;
	const uint32_t align = getValidatedAlign(cmdHead->align);
	const size_t byteCountToWrite = payloadSize<MemReadWriteCmdHead>(inSize, requiredWriteSize, align);
	const uint8_t* const dataToWrtie = inBuf + sizeof(MemReadWriteCmdHead);
	if (isFlashRegion(address)){
		flashWrite(address, dataToWrtie, byteCountToWrite);
	} else {
		dbg_memcpy( reinterpret_cast<uint8_t*>(address), inBuf + sizeof(MemReadWriteCmdHead), byteCountToWrite, align );
	}
	MemReadWriteAnsHead* const ansHead = reinterpret_cast<MemReadWriteAnsHead* const>(outBuf);
	ansHead->cmd = static_cast<uint8_t>(DebugCommand::MemoryWrite);
	ansHead->byteCount = byteCountToWrite;
	ansHead->address = address;
	ansHead->align = align;
	return sizeof(MemReadWriteAnsHead);
}

size_t regsReadCommand(const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize){
	if ( !dbgHeadValid<RegReadWriteCmdHead>( inBuf, inSize, DebugCommand::RegsRead) ) return 0;
	const RegReadWriteCmdHead* const cmdHead = reinterpret_cast<const RegReadWriteCmdHead* const>(inBuf);
	const size_t bytesToRead = payloadSize<RegReadWriteAnsHead>(outSize, cmdHead->regCount * sizeof(uint32_t), sizeof(uint32_t) );
	size_t regsToRead = bytesToRead / sizeof(uint32_t);
	const size_t regOffset = cmdHead->regIndex;
	const size_t maxRegIndex = sizeof(ArmRegisters) / sizeof(uint32_t);
	if (regOffset > maxRegIndex) return 0;
	const size_t maxRegsToRead = maxRegIndex - regOffset + 1;
	regsToRead = (regsToRead < maxRegsToRead) ? regsToRead : maxRegsToRead;
	volatile  const ArmRegisters* const regs = debugGetRegs();
	uint32_t* const regsTo = reinterpret_cast<uint32_t* const>(outBuf + sizeof(RegReadWriteAnsHead));
	for(size_t index = 0; index < regsToRead; index++){
		regsTo[index] = regs->r[index + regOffset];
	}
	RegReadWriteAnsHead* const ansHead = reinterpret_cast<RegReadWriteAnsHead* const>(outBuf);
	ansHead->cmd = static_cast<uint8_t>(DebugCommand::RegsRead);
	ansHead->regCount = regsToRead;
	ansHead->regIndex = regOffset;
	return ( sizeof(RegReadWriteAnsHead) + bytesToRead );
}

size_t regsWriteCommand(const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize){
	if ( !dbgHeadValid<RegReadWriteCmdHead>( inBuf, inSize, DebugCommand::RegsWrite) ) return 0;
	const RegReadWriteCmdHead* const cmdHead = reinterpret_cast<const RegReadWriteCmdHead* const>(inBuf);
	const size_t bytesToWrite = payloadSize<RegReadWriteCmdHead>(inSize, cmdHead->regCount * sizeof(uint32_t), sizeof(uint32_t) );
	size_t regsToWrite = bytesToWrite / sizeof(uint32_t);
	const size_t regOffset = cmdHead->regIndex;
	const size_t maxRegIndex = sizeof(ArmRegisters) / sizeof(uint32_t);
	if (regOffset > maxRegIndex) return 0;
	const size_t maxRegsToWrite = maxRegIndex - regOffset + 1;
	regsToWrite = (regsToWrite < maxRegsToWrite) ? regsToWrite : maxRegsToWrite;
	volatile ArmRegisters* const regs = debugGetRegs();
	const uint32_t* const regsFrom = reinterpret_cast<const uint32_t*>(inBuf + sizeof(RegReadWriteCmdHead));
	for(size_t index = 0; index < regsToWrite; index++){
		regs->r[index + regOffset] = regsFrom[index];
	}
	RegReadWriteAnsHead* const ansHead = reinterpret_cast<RegReadWriteAnsHead* const>(outBuf);
	ansHead->cmd = static_cast<uint8_t>(DebugCommand::RegsWrite);
	ansHead->regCount = regsToWrite;
	ansHead->regIndex = regOffset;
	return sizeof(RegReadWriteAnsHead);
}


size_t resetHaltCommand(const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize){
	if ( !dbgHeadValid<ResetHaltResumeCmdHead>( inBuf, inSize, DebugCommand::ResetHalt) ) return 0;
	const ResetHaltResumeCmdHead* const cmdHead = reinterpret_cast<const ResetHaltResumeCmdHead* const>(inBuf);
	const ResetHaltCommand cmd = static_cast<ResetHaltCommand>(cmdHead->haltCommand);
	ResetHaltResumeAnsHead* const ansHead = reinterpret_cast<ResetHaltResumeAnsHead* const>(outBuf);

	ansHead->cmd = static_cast<uint8_t>(DebugCommand::ResetHalt);
	ansHead->haltCommand = cmdHead->haltCommand;
	ansHead->reserved = 0;
	ansHead->haltState = static_cast<uint8_t>(ResetHaltState::Running);

	switch(cmd){
	case ResetHaltCommand::GetState:
		ansHead->haltState = static_cast<uint8_t>( debugGetHaltState() );
	break;
	case ResetHaltCommand::HaltOnly:
		debugHaltRequest();
	break;
	case ResetHaltCommand::ResetAndHalt:
		debugHaltRequest();
	break;
	case ResetHaltCommand::ResetOnly:
		debugHaltRequest();
	break;
	case ResetHaltCommand::Resume:
		debugResume(false);
	break;
	case ResetHaltCommand::ResumeStep:
		debugResume(true);
	}

	return sizeof(ResetHaltResumeAnsHead);
}


size_t flashCommand(const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize){
	if ( !dbgHeadValid<FlashCmdHead>( inBuf, inSize, DebugCommand::Flash) ) return 0;
	const FlashCmdHead* const cmdHead = reinterpret_cast<const FlashCmdHead* const>(inBuf);
	const FlashCommand cmd = static_cast<FlashCommand>(cmdHead->flashCommand);

	FlashAnsHead* const ansHead = reinterpret_cast<FlashAnsHead* const>(outBuf);
	ansHead->cmd = static_cast<uint8_t>(DebugCommand::Flash);
	ansHead->flashCommand = cmdHead->flashCommand;
	ansHead->status = 0;

	switch(cmd){

	case FlashCommand::Detach:
		asm volatile ("isb; dsb;");
		debugStartFirmware();
		ansHead->status = 1;
	break;

	case FlashCommand::Erase:
	{

		debugStartEraseLoop();
#if 0
		asm volatile ("isb; dsb;");
		debugStartLocalLoop();
		uint32_t addrFrom = cmdHead->address;
		uint32_t addrTo = addrFrom + cmdHead->size;
		flash_unlock();
		for(uint32_t address = addrFrom; address < addrTo; address += 0x100){
			uint32_t pageAddress = address;
			flash_erase_page(pageAddress + 4);
			flash_erase_page(pageAddress + 4);
		}
#endif
		ansHead->status = 1;
	}
	break;

	case FlashCommand::Lock:
	break;

	case FlashCommand::Unlock:
		asm volatile ("isb; dsb;");
		debugStartLocalLoop();
		flash_unlock();
		ansHead->status = 1;
	break;

	}

	return sizeof(FlashAnsHead);
}

size_t breakpointCommand(const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize){
	if ( !dbgHeadValid<BreakpointCmdHead>( inBuf, inSize, DebugCommand::Breakpoint) ) return 0;
	const BreakpointCmdHead* const cmdHead = reinterpret_cast<const BreakpointCmdHead* const>(inBuf);
	BreakpointAnsHead* const ansHead = reinterpret_cast<BreakpointAnsHead* const>(outBuf);
	ansHead->cmd = static_cast<uint8_t>(DebugCommand::Breakpoint);
	ansHead->count = cmdHead->count;
	ansHead->set = cmdHead->set;
	const bool set = cmdHead->set;

	if (set){
		// Set breakpoints
		FPB->FP_CTRL |= 0x03;
		for(size_t index = 0; index < cmdHead->count; index++){
			const Breakpoint* breakpoint = reinterpret_cast<const Breakpoint*>(&inBuf[sizeof(BreakpointCmdHead)]);
			uint32_t address = breakpoint[index].address & 0x3FFFFFFC;
			uint32_t addrlow = (breakpoint[index].address & 0x02) ? 0x02 : 0x01;
			bool success = false;
			for(uint32_t bpNum = 0; bpNum < NUMOFBKPTS; bpNum++){
				volatile uint32_t* bp = &(FPB->FP_COMP[bpNum]);
				if ( ((*bp) & 0x3FFFFFFC) == address) {
					*bp  |= 0x01 | (addrlow << 30);
					success = true;
					break;
				}
			}
			if (!success){
				for(uint32_t bpNum = 0; bpNum < NUMOFBKPTS; bpNum++){
					volatile uint32_t* bp = &(FPB->FP_COMP[bpNum]);
					if ( ((*bp) & 0x01) == 0) {
						*bp  = address | (0x01 | (addrlow << 30));
						success = true;
						break;
					}
				}
			}
		}

	} else {
		for(size_t index = 0; index < cmdHead->count; index++){
			const Breakpoint* breakpoint = reinterpret_cast<const Breakpoint*>( &inBuf[sizeof(BreakpointCmdHead)]);
			uint32_t address = breakpoint[index].address & 0x3FFFFFFC;
			uint32_t addrlow = (breakpoint[index].address & 0x02) ? 0x02 : 0x01;
			for(uint32_t bpNum = 0; bpNum < NUMOFBKPTS; bpNum++){
				volatile uint32_t* bp = &FPB->FP_COMP[bpNum];
				if ( ((*bp) & 0x3FFFFFFC) == address) {
					*bp  &= ~(addrlow << 30);
					if ((*bp & 0xC0000000) == 0) *bp = 0;
				}
			}
		}
	}
	return sizeof(BreakpointAnsHead);
}

size_t attachCommand(const uint8_t* const inBuf, const size_t inSize, uint8_t* const outBuf, const size_t outSize){
	if ( !dbgHeadValid<AttachDetachCmdHead>( inBuf, inSize, DebugCommand::Attach) ) return 0;
	const AttachDetachCmdHead * const cmdHead = reinterpret_cast<const AttachDetachCmdHead* const>(inBuf);
	AttachDetachAnsHead * const ansHead = reinterpret_cast<AttachDetachAnsHead* const>(outBuf);
	ansHead->cmd = cmdHead->cmd;
	ansHead->attach = cmdHead->attach;
	ansHead->state = true;
	AttachCommand cmd = static_cast<AttachCommand>(cmdHead->attach);
	if (cmd == AttachCommand::Attach){
		debugHaltRequest();
	} if (cmd == AttachCommand::Detach) {
		debugStartFirmware();
		debugResume(false);
	}
	return sizeof(AttachDetachAnsHead);
}

#include "Usb.h"
extern usbd_device *usbDevice;


void usbDataOutCallback(usbd_device *usbd_dev, uint8_t ep){
	uint16_t rcvSize = usbd_ep_read_packet(usbd_dev, ep, usbCmdFrame, BULK_EP_MAXPACKET);
	if ( 0 == rcvSize ) return;

	DebugCommand command =  static_cast<DebugCommand>(usbCmdFrame[0]);
	size_t retSize = 0;

	switch(command){
	case DebugCommand::MemoryRead:
		retSize = memoryReadCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;
	case DebugCommand::MemoryWrite:
		retSize = memoryWriteCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;
	case DebugCommand::RegsRead:
		retSize = regsReadCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;
	case DebugCommand::RegsWrite:
		retSize = regsWriteCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;
	case DebugCommand::ResetHalt:
		retSize = resetHaltCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;
	case DebugCommand::Breakpoint:
		retSize = breakpointCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;
	case DebugCommand::Flash:
		retSize = flashCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;
	case DebugCommand::Attach:
		retSize = attachCommand(usbCmdFrame, rcvSize, usbAnsFrame, BULK_EP_MAXPACKET);
	break;

	default:
		break;
	}

	if (retSize == 0){
		volatile bool loop = true;
		while(loop){
			asm("nop");
		}
	}

	usbd_ep_write_packet(usbd_dev, ep | 0x80, usbAnsFrame, retSize);
}


extern "C" void systemMain(){
	asm volatile ("movs r3, #0; svc 0;");
	while( true ){
		asm volatile ("nop");
		asm volatile ("nop");
	}
}

