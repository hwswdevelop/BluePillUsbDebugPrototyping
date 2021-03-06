
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
#include "TargetInterface/WinUsbTargetInterface.h"
#include "Target/TargetSTM32F1.h"

#if 0

void stm32_regs_read(target* t, void* data) {
	uint32_t* regs = (uint32_t*)data;
	stm32RegsRead(0, 8, regs);
	stm32RegsRead(8, 8, regs + 8);
}

size_t stm32_reg_read(target* t, int reg, void* data, size_t max) {
	static const int REG_SP = 13;
	static const int REG_xPSR = 0x10;
	static const int REG_CPPSR = 0x19;
	static const int REG_MSP = 0x11;
	static const int REG_PSP = 0x12;
	if (reg <= REG_xPSR) {
		stm32RegsRead(reg, 1, data);
		return sizeof(uint32_t);
	}
	else if ((reg == REG_MSP) || (reg == REG_PSP)) {
		stm32RegsRead(REG_SP, 1, data);
		return sizeof(uint32_t);
	}
	else if (reg == REG_CPPSR) {
		stm32RegsRead(REG_xPSR, 1, data);
		return sizeof(uint32_t);
	}
	else {
		printf("**************** Faild to read reg 0x%08X *******************\n", reg);
		memset(data, 0x00, max);
		//*((uint32_t*)data) = reg | 0xFF000000;
		return sizeof(uint32_t);
	}
}

int stm32_mem_read(target* t, void* dest, target_addr src, size_t len) {
	return stm32MemRead(src, dest, len);
}


size_t stm32_reg_write(target* t, int reg, const void* data, size_t size) {
	static const int REG_SP = 13;
	static const int REG_xPSR = 0x10;
	static const int REG_CPPSR = 0x19;
	static const int REG_MSP = 0x11;
	static const int REG_PSP = 0x12;
	if (reg <= REG_xPSR) {
		stm32RegsWrite(reg, 1, data);
		return sizeof(uint32_t);
	}
	else if ((reg == REG_MSP) || (reg == REG_PSP)) {
		stm32RegsWrite(reg, 1, data);
		return sizeof(uint32_t);
	}
	else if (reg == REG_CPPSR) {
		stm32RegsWrite(reg, 1, data);
		return sizeof(uint32_t);
	}
	else {
		printf("**************** Faild to write reg 0x%08X *******************\n", reg);
		return sizeof(uint32_t);
	}
}

void stm32_mem_write(target* t, target_addr dest, const void* src, size_t len) {
	stm32MemWrite(dest, src, len);
}

void stm32_halt_resume(target* t, bool step) {
	printf("*********************************************\n");
	printf("*** HaltResume step = %d\n", step);
	printf("********************************************\n");
	stm32HaltResume(step);
}

void stm32_halt_request(target* t) {
	stm32HaltRequest();
}

bool stm32_check_error(target* t) {
	return false;
}

int stm32_breakwatch_set(target* t, struct breakwatch* bw) {
	struct breakwatch* current = bw;
	uint32_t count = 0;
	while (bw != NULL) {
		printf("*********************************************\n");;
		printf("*** Enable breakpoint at 0x%X size = %d\n", bw->addr, bw->size);
		printf("*********************************************\n");
		count += stm32BreakpointSetClear(bw->addr, bw->type, 1);
		bw = bw->next;
	}
	return (count > 0) ? 0 : -1;
}

int stm32_breakwatch_clear(target* t, struct breakwatch* bw) {
	struct breakwatch* current = bw;
	uint32_t count = 0;
	while (bw != NULL) {
		printf("*********************************************\n");
		printf("*** Disable breakpoint at 0x%X size = %d\n", bw->addr, bw->size);
		printf("********************************************\n");
		count += stm32BreakpointSetClear(bw->addr, bw->type, 0);
		bw = bw->next;
	}
	return (count > 0) ? 0 : -1;
}

enum target_halt_reason stm32_halt_poll(target* t, target_addr* watch) {
	*watch = 0;
	enum target_halt_reason reason = stm32HaltPoll();
	if (reason != TARGET_HALT_RUNNING) {
		printf("*********************************************\n");
		printf(" Halted\n");
		printf("*********************************************\n");
	}
	return reason;
}

bool stm32_attach(target* t) {
	return stm32AttachCmd();
}

void stm32_detach(target* t) {
	stm32DetachCmd();
}

void (stm32_reset)(target* t) {
	stm32Reset();
}

#endif

void gdbInit() {
	target_list_free();




	target* t = target_new();
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
	stm32HaltRequest();
	stm32f1_probe(t);
	//target_attach(t, &gdb_controller);
	set_current_target(t);
}