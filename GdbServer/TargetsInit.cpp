
#include "general.h"
#include "ctype.h"
#include "hex_utils.h"
#include "gdb_if.h"
#include "gdb_packet.h"
#include "gdb_main.h"
#include "gdb_hostio.h"
#include "target.h"
#include "command.h"
#include "crc32.h"
#include "morse.h"
#include "platform.h"
#include "target_internal.h"
#include "target.h"
#include "TargetInterface/WinUsbTargetInterface.h"
#include "Target/TargetSTM32F1.h"



extern "C" void stm32_regs_read(target* t, void* data) {
	DEBUG_TARGET("stm32_regs_read\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	uint32_t* regs = (uint32_t*)data;
	auto readRegs = device->regsRead(0, 16);
	std::copy(readRegs.begin(), readRegs.end(), reinterpret_cast<uint8_t*>(data) );
	readRegs.clear();
}

extern "C" size_t stm32_reg_read(target* t, int reg, void* data, size_t max) {
	DEBUG_TARGET("stm32_reg_read index = %d, count=%d\n", reg, max/sizeof(uint32_t));
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	static const int REG_SP = 13;
	static const int REG_xPSR = 0x10;
	static const int REG_CPPSR = 0x19;
	static const int REG_MSP = 0x11;
	static const int REG_PSP = 0x12;
	if (reg <= REG_xPSR) {
		auto readRegs = device->regsRead(reg, 1);
		std::copy(readRegs.begin(), readRegs.end(), reinterpret_cast<uint8_t*>(data));
		readRegs.clear();
		return sizeof(TargetRegister);
	}
	else if ((reg == REG_MSP) || (reg == REG_PSP)) {
		auto readRegs = device->regsRead(REG_SP, 1);
		std::copy(readRegs.begin(), readRegs.end(), reinterpret_cast<uint8_t*>(data));
		readRegs.clear();
		return sizeof(TargetRegister);
	}
	else if (reg == REG_CPPSR) {
		auto readRegs = device->regsRead(REG_xPSR, 1);
		std::copy(readRegs.begin(), readRegs.end(), reinterpret_cast<uint8_t*>(data));
		readRegs.clear();
		return sizeof(TargetRegister);
	}
	else {
		DEBUG_PROBE("**************** Faild to read reg 0x%08X *******************\n", reg);
		memset(data, 0x00, max);
		return sizeof(TargetRegister);
	}
}

extern "C" void stm32_mem_read(target* t, void* dest, target_addr src, size_t len) {
	DEBUG_TARGET("stm32_mem_read address=0x%X, size=%d\n", (uint32_t)src, len);
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);	
	size_t addrAccSize = (src & 0x01) ? 1 : ((src & 0x02) ? 2 : 4);
	size_t szAccSize = (len & 0x01) ? 1 : ((len & 0x02) ? 2 : 4);
	size_t accSize = (addrAccSize < szAccSize) ? addrAccSize : szAccSize;
	auto readMem = device->memRead(src, accSize, static_cast<AccessSize>(len / accSize) );
	std::copy(readMem.begin(), readMem.end(), reinterpret_cast<uint8_t*>(dest) );
	readMem.clear();
}


extern "C" size_t stm32_reg_write(target* t, int reg, const void* data, size_t size) {
	DEBUG_TARGET("stm32_reg_write index = %d, count=%d\n", reg, size / sizeof(uint32_t));
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	static const int REG_SP = 13;
	static const int REG_xPSR = 0x10;
	static const int REG_CPPSR = 0x19;
	static const int REG_MSP = 0x11;
	static const int REG_PSP = 0x12;

	std::vector<TargetRegister> regs;
	regs.assign(reinterpret_cast<TargetRegister*>(&data), reinterpret_cast<TargetRegister*>(&data) + (size / sizeof(TargetRegister)));

	if (reg <= REG_xPSR) {
		device->regsWrite(reg, regs);
		regs.clear();
		return sizeof(uint32_t);
	}
	else if ((reg == REG_MSP) || (reg == REG_PSP)) {
		device->regsWrite(REG_SP, regs);
		regs.clear();
		return sizeof(uint32_t);
	}
	else if (reg == REG_CPPSR) {
		device->regsWrite(REG_xPSR, regs);
		regs.clear();
		return sizeof(uint32_t);
	}
	else {
		regs.clear();
		DEBUG_PROBE("**************** Faild to write reg 0x%08X *******************\n", reg);
		return sizeof(uint32_t);
	}
}

extern "C" void stm32_mem_write(target* t, target_addr dest, const void* src, size_t len) {
	DEBUG_TARGET("stm32_mem_write address=0x%X, size=%d\n", (uint32_t)dest, len);
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	std::vector<uint8_t> data;
	data.assign( reinterpret_cast<uint8_t*>(&data), reinterpret_cast<uint8_t*>(&data) + len );
	size_t addrAccSize = (dest & 0x01) ? 1 : ((dest & 0x02) ? 2 : 4);
	size_t szAccSize = (len & 0x01) ? 1 : ((len & 0x02) ? 2 : 4);
	size_t accSize = (addrAccSize < szAccSize) ? addrAccSize : szAccSize;
	device->memWrite(dest, accSize, data);
	data.clear();
}

extern "C" void stm32_halt_resume(target* t, bool step) {
	DEBUG_TARGET("stm32_halt_resume step = %d\n", step);
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	DEBUG_PROBE("*********************************************\n");
	DEBUG_PROBE("*** HaltResume step = %d\n", step);
	DEBUG_PROBE("********************************************\n");
	if (step){
		device->resetHaltResume(ResetHaltCommand::ResumeStep);
	} else {
		device->resetHaltResume(ResetHaltCommand::Resume);
	}
}

extern "C" void stm32_halt_request(target* t) {
	DEBUG_TARGET("stm32_halt_request\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	device->resetHaltResume(ResetHaltCommand::HaltOnly);
}

extern "C" bool stm32_check_error(target* t) {
	DEBUG_TARGET("stm32_check_error\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	return false;
}

extern "C" int stm32_breakwatch_set(target* t, struct breakwatch* bw) {
	DEBUG_TARGET("stm32_breakwatch_set\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	struct breakwatch* current = bw;
	uint32_t count = 0;
	std::vector<Breakpoint> bp(32);
	while (bw != NULL) {
		DEBUG_PROBE("*********************************************\n");;
		DEBUG_PROBE("*** Enable breakpoint at 0x%X size = %d\n", bw->addr, bw->size);
		DEBUG_PROBE("*********************************************\n");
		bp[count].address = bw->addr;
		bp[count].size = static_cast<uint8_t>(bw->size);
		bp[count].type = (BreakpointType)bw->type;
		bp[count].reserved = 0;
		count++;
		bw = bw->next;
	}
	bp.resize(count);
	device->breakpointsSetClear(bp, true);
	bp.clear();
	return (count > 0) ? 0 : -1;
}

extern "C" int stm32_breakwatch_clear(target* t, struct breakwatch* bw) {
	DEBUG_TARGET("stm32_breakwatch_clear\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	struct breakwatch* current = bw;
	uint32_t count = 0;
	std::vector<Breakpoint> bp(128);
	while (bw != NULL) {
		DEBUG_PROBE("*********************************************\n");
		DEBUG_PROBE("*** Disable breakpoint at 0x%X size = %d\n", bw->addr, bw->size);
		DEBUG_PROBE("********************************************\n");
		bp[count].address = bw->addr;
		bp[count].size = static_cast<uint8_t>(bw->size);
		bp[count].type = (BreakpointType)bw->type;
		bp[count].reserved = 0;
		count++;
		bw = bw->next;
	}
	bp.resize(count);
	device->breakpointsSetClear(bp, false);
	bp.clear();
	return (count > 0) ? 0 : -1;
}

extern "C" enum target_halt_reason stm32_halt_poll(target* t, target_addr* watch) {
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	*watch = 0;
	enum target_halt_reason reason = TARGET_HALT_RUNNING;
	reason = (enum target_halt_reason)device->resetHaltResume(ResetHaltCommand::GetState);

	if (reason != TARGET_HALT_RUNNING) {
		DEBUG_PROBE("*********************************************\n");
		DEBUG_PROBE(" Halted\n");
		DEBUG_PROBE("*********************************************\n");
	}
	return reason;
}

extern "C" bool stm32_attach(target* t) {
	DEBUG_TARGET("stm32_attach\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	device->attachDetach(AttachCommand::Attach);
	return true;
}

extern "C" void stm32_detach(target* t) {
	DEBUG_TARGET("stm32_detach\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	device->attachDetach(AttachCommand::Detach);
}

extern "C" void stm32_reset(target* t) {
	DEBUG_TARGET("stm32_reset\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	device->resetHaltResume(ResetHaltCommand::ResetAndHalt);
}

extern "C" void stm32_flash_unlock(target * t, uint32_t address, size_t size) {
	DEBUG_TARGET("stm32_flash_unlock address = 0x%X size=%d\n", address, size);
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	device->flashCmd(FlashCommand::Unlock, address, size);
}

extern "C" void stm32_flash_erase(target * t, uint32_t address, size_t size) {
	DEBUG_TARGET("stm32_flash_erase address = 0x%X size=%d\n", address, size);
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	device->flashCmd(FlashCommand::Erase, address, size);
}

extern "C" void stm32_flash_wrtie(target * t, const uint32_t address, const uint8_t* data, const size_t size) {
	DEBUG_TARGET("stm32_flash_wrtie address = 0x%X size=%d\n", address, size);
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	std::vector<uint8_t> flashData;
	flashData.assign(data, data + size);
	device->memWrite(address, 2, flashData);
}

extern "C" void stm32_flash_done(target * t) {
	DEBUG_TARGET("stm32_flash_done\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	device->flashCmd(FlashCommand::Detach, 0, 0);
}

extern "C" void stm32_priv_delete(void* priv) {
	DEBUG_TARGET("stm32_priv_delete\n");
	TargetSTM32F1* device = (TargetSTM32F1*)(priv);
	delete device;
}

WinUsbTargetInterfaceFabric* targetFabric = nullptr;

extern "C" bool stm32f1_probe(target * t);

#include <thread>
#include <iostream>

void getSerialDataThread(target* t) {
	TargetSTM32F1* device = (TargetSTM32F1*)(t->priv);
	while (true) {
		std::string data = device->getSerialData();
		std::cout << data;
	}
}

extern "C" void gdbInit() {
	target_list_free();

	targetFabric = new WinUsbTargetInterfaceFabric();
	targetFabric->updateInterfaces();
	size_t count = targetFabric->getInterfaceCount();
	for (size_t index = 0; index < count; index++) {
		TargetInterface* targetInterface = targetFabric->getInterface(index);
		target* t = target_new();
		auto device = new TargetSTM32F1(*targetInterface);
		t->priv = (void*)device;
		t->priv_free = stm32_priv_delete;
		t->regs_size = 16 * sizeof(uint32_t);
		t->halt_poll = stm32_halt_poll;
		t->regs_read = stm32_regs_read;
		t->reg_read = stm32_reg_read;
		t->reg_write = stm32_reg_write;
		t->mem_read = stm32_mem_read;
		t->mem_write = stm32_mem_write;
		t->halt_resume = stm32_halt_resume;
		t->halt_request = stm32_halt_request;
		t->breakwatch_set = stm32_breakwatch_set;
		t->breakwatch_clear = stm32_breakwatch_clear;
		t->attach = stm32_attach;
		t->detach = stm32_detach;
		t->reset = stm32_reset;
		t->extended_reset = stm32_reset;
		t->check_error = stm32_check_error;
		set_current_target(t);
		stm32f1_probe(t);
		t->halt_request(t);
		t->halt_request(t);
		std::thread* serialThread = new std::thread(getSerialDataThread, t);
		serialThread->detach();
	}
}