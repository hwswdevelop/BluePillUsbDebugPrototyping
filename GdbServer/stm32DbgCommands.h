#pragma once


#include "DbgCommands.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t stm32MemRead(uint32_t address, uint8_t* data, size_t size);
size_t stm32MemWrite(uint32_t address, uint8_t * data, size_t size);
size_t stm32RegsRead(uint8_t regIndex, uint8_t count, uint8_t * buf);
size_t stm32RegsWrite(uint8_t regIndex, uint8_t count, uint32_t values[]);
size_t stm32HaltResume(uint8_t step);
size_t stm32HaltRequest();
uint8_t stm32HaltPoll();
uint8_t stm32BreakpointSetClear(uint32_t address, uint8_t type, uint8_t set);
size_t stm32FlashEraseCmd(uint32_t address, uint16_t size);
size_t stm32FlashWriteCmd(uint32_t address, uint8_t* data, uint16_t size);
bool  stm32AttachCmd();
void  stm32DetachCmd();
void  stm32FlashDone();
void  stm32Reset();

#ifdef __cplusplus
}
#endif
