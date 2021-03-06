/*
 * Debug.cpp
 *
 *  Created on: 7 февр. 2021 г.
 *      Author: Evgeny
 */

#include "Debug.h"
#include <cstdint>
#include <cstddef>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>


#include "CircularBuffer.h"

enum class DebugHandlerCommand : uint8_t {
	Nop,
	Halt,
	Resume,
	ResumeStep,
	StartFirmware,
	StartLocalLoop,
	StartEraseLoop
};


struct FlashEraseParams {
	uint32_t address;
	uint32_t size;
};

extern ArmRegisters context_data;
static CircularBuffer<FlashEraseParams, 4> gEraseParams {};
static CircularBuffer<DebugHandlerCommand, 16> gCmdQueue {};
static volatile ArmRegisters* const gDebugRegisters = &context_data;
static volatile ResetHaltState gHaltState = ResetHaltState::Running;

extern "C" bool debugHaltRequest(){
	bool result = gCmdQueue.put(DebugHandlerCommand::Halt);
	DEMCR |= DEMCR_MON_PEND;
	return result;
}

extern "C" bool debugResume(bool step){
	bool result = gCmdQueue.put((step ? DebugHandlerCommand::ResumeStep : DebugHandlerCommand::Resume));
	DEMCR |= DEMCR_MON_PEND;
	return result;
}

extern "C" bool debugStartFirmware(){
	bool result = gCmdQueue.put(DebugHandlerCommand::StartFirmware);
	DEMCR |= DEMCR_MON_PEND;
	return result;
}

extern "C" bool debugStartLocalLoop(){
	bool result = gCmdQueue.put(DebugHandlerCommand::StartLocalLoop);
	DEMCR |= DEMCR_MON_PEND;
	return result;
}

extern "C" bool debugStartEraseLoop(uint32_t address, uint32_t size){
	const  FlashEraseParams params = { .address = address, .size = size};
	bool result = gEraseParams.put(params);
	result &= gCmdQueue.put(DebugHandlerCommand::StartEraseLoop);
	DEMCR |= DEMCR_MON_PEND;
	return result;
}

extern "C" volatile ArmRegisters* const debugGetRegs(){
	return gDebugRegisters;
}

extern "C" ResetHaltState debugGetHaltState(){
	return gHaltState;
}


void  debugLocalLoop(){
	while(true){
		asm volatile("nop");
		asm volatile("nop");
	}
}

void debugResetUserPheripherals(){
	rcc_periph_reset_pulse(RST_TIM2);
	rcc_periph_reset_pulse(RST_TIM3);
	rcc_periph_reset_pulse(RST_TIM4);
	rcc_periph_reset_pulse(RST_WWDG);
	rcc_periph_reset_pulse(RST_SPI2);
	rcc_periph_reset_pulse(RST_USART2);
	rcc_periph_reset_pulse(RST_USART3);
	rcc_periph_reset_pulse(RST_I2C1);
	rcc_periph_reset_pulse(RST_I2C2);
	rcc_periph_reset_pulse(RST_CAN);
	rcc_periph_reset_pulse(RST_GPIOB);
	rcc_periph_reset_pulse(RST_GPIOD);
	rcc_periph_reset_pulse(RST_GPIOE);
	rcc_periph_reset_pulse(RST_ADC1);
	rcc_periph_reset_pulse(RST_ADC2);
	rcc_periph_reset_pulse(RST_TIM1);
	rcc_periph_reset_pulse(RST_SPI1);
	rcc_periph_reset_pulse(RST_USART1);
}


void debugDisableUserInterrupts(){
	for(uint32_t irqNo = 0; irqNo < 256 ; irqNo++){
		if (irqNo == 19) continue;
		if (irqNo == 20) continue;
		nvic_disable_irq(irqNo);
	}
	debugResetUserPheripherals();
	for(uint32_t irqNo = 0; irqNo < 256 ; irqNo++){
		if (irqNo == 19) continue;
		if (irqNo == 20) continue;
		nvic_clear_pending_irq(irqNo);
	}
}

static constexpr const uint32_t userCodeVectorTable = 0x8004000;
static constexpr const uint32_t resetVectorOffset = 4;
static constexpr const uint32_t maxFirmwareSize = 0x8020000 - userCodeVectorTable;

void enableDisableDebugCommandHandler(bool enable){
	if (enable){
		nvic_enable_irq(19);
		nvic_enable_irq(20);
	} else {
		nvic_disable_irq(19);
		nvic_disable_irq(20);
	}

}
#include <libopencm3/stm32/flash.h>
void flashEraseLoop(){
	enableDisableDebugCommandHandler(false);
	FlashEraseParams params;
	while(gEraseParams.pull(params)){
		const uint32_t address = params.address;
		const uint32_t size = params.size;
		const uint32_t lastAddress = address + size;
		if ( (address < 0x8004000) || (address > 0x80020000) ) break;
		if ( lastAddress > 0x80020000 ) break;
		for(uint32_t curAddress = address; curAddress < lastAddress; curAddress += 0x400){
			bool erase = false;
			for(uint32_t checkAddress = curAddress; checkAddress < (curAddress + 0x400); checkAddress +=4 ){
				uint32_t value = *((uint32_t*)checkAddress);
				if (value != 0xFFFFFFFF){
					erase = true;
					break;
				}
			}
			if (erase) {
				flash_unlock();
				flash_erase_page(curAddress);
			}
		}
	}
	enableDisableDebugCommandHandler(true);
	debugStartLocalLoop();
	while(true){
		asm volatile("nop");
		asm volatile("nop");
	}
}

extern volatile bool allowUsbTransfer;

extern "C"  void DebugHandler(ArmRegisters* regs){
	static uint8_t stepOnThread = 0;
	static bool exitFromInterrupt = false;
	bool interruptActive = (regs->xpsr & 0xFF);

	if (interruptActive){

		if (stepOnThread != 0){
			if ( DFSR & DFSR_BKPT ) {
				stepOnThread = 0;
				exitFromInterrupt = false;
				DEMCR &= ~DEMCR_MON_STEP;
				asm volatile("movs r0, #0x00; msr BASEPRI, r0; isb;");
			}
		}

		if (stepOnThread == 1){
			stepOnThread = 2;
			exitFromInterrupt = true;
			asm volatile("movs r0, #0x30; msr BASEPRI, r0; isb;");
		}

		if (exitFromInterrupt){
			DEMCR |= DEMCR_MON_STEP;
			DEMCR &= ~DEMCR_MON_PEND;
			return;
		}

	} else {
		if (stepOnThread == 2){
			stepOnThread = 3;
			DEMCR |= DEMCR_MON_STEP;
			DEMCR &= ~DEMCR_MON_PEND;
			return;
		}

		if (stepOnThread == 3){
			stepOnThread = 0;
			asm volatile("movs r0, #0x00; msr BASEPRI, r0; isb;");
		}

		if (exitFromInterrupt){
			DEMCR &= ~DEMCR_MON_STEP;
			exitFromInterrupt = false;
		}
	}

	stepOnThread = 0;

	setDebugCode(1);

	if ( DEMCR & DEMCR_MON_REQ ){
		// Request
		DEMCR &= ~DEMCR_MON_PEND;
		if (gCmdQueue.isEmpty()){
			return;
		}
	} else {
		if (gCmdQueue.isEmpty()){
			gHaltState = ResetHaltState::Breakpoint;
		}
	}

	setDebugCode(2);


	DebugHandlerCommand cmd {};

	do {

		if ( gCmdQueue.get(cmd) ){

			switch(cmd){
			case DebugHandlerCommand::Halt:
				 gCmdQueue.pull(cmd);
				gHaltState = ResetHaltState::Request;
			break;

			case DebugHandlerCommand::Resume:
			{
				gCmdQueue.pull(cmd);
				DEMCR &= (~DEMCR_MON_STEP);
				gHaltState = ResetHaltState::Running;
			}
			break;

			case DebugHandlerCommand::ResumeStep:
			{
				// Enable stepping
				gCmdQueue.pull(cmd);
				if (!interruptActive){
					stepOnThread = 1;

				}
				DFSR |= DFSR_BKPT;
				DEMCR |= DEMCR_MON_STEP;
				DEMCR &= ~DEMCR_MON_PEND;
				gHaltState = ResetHaltState::Running;
			}
			break;

			case DebugHandlerCommand::StartLocalLoop:
			{
				setDebugCode(3);
				if (!interruptActive){
					gCmdQueue.pull(cmd);
					debugDisableUserInterrupts();
					regs->common.pc = (uint32_t)debugLocalLoop;
					regs->common.lr = regs->common.pc;
					regs->common.sp = 0x20005000;
					DEMCR |= DEMCR_MON_STEP;
					regs->xpsr = (regs->common.pc & 0x01) << 24;
					DEMCR |= DEMCR_MON_STEP;
					DEMCR &= ~DEMCR_MON_PEND;
					gHaltState = ResetHaltState::Running;
					return;
				} else {
					exitFromInterrupt = true;
					DEMCR |= DEMCR_MON_STEP;
					DEMCR &= ~DEMCR_MON_PEND;
					return;

					setDebugCode(0x0F);
					while(true){};
				}
			}
			break;

			case DebugHandlerCommand::StartEraseLoop:
			{
				setDebugCode(3);
				if (!interruptActive){
					gCmdQueue.pull(cmd);
					debugDisableUserInterrupts();
					regs->common.pc = (uint32_t)flashEraseLoop;
					regs->common.lr = regs->common.pc;
					regs->common.sp = 0x20005000;
					regs->xpsr = (regs->common.pc & 0x01) << 24;
					DEMCR &= ~DEMCR_MON_PEND;
					gHaltState = ResetHaltState::Running;
					return;
				} else {
					exitFromInterrupt = true;
					DEMCR |= DEMCR_MON_STEP;
					DEMCR &= ~DEMCR_MON_PEND;
					return;

					setDebugCode(0x0F);
					while(true){};
				}
			}
			break;

			case DebugHandlerCommand::StartFirmware:
			{
				setDebugCode(4);
				if (!interruptActive){
					gCmdQueue.pull(cmd);
					debugDisableUserInterrupts();
					const uint32_t stackPointer = *(uint32_t*)( userCodeVectorTable );
					const uint32_t resetAddress = *(uint32_t*)( userCodeVectorTable + resetVectorOffset );
					const uint32_t lastFirmwareAddress = userCodeVectorTable + maxFirmwareSize;

					for(uint32_t addr = userCodeVectorTable ; addr < lastFirmwareAddress; addr += 4){
						volatile uint32_t temp = *(volatile uint32_t*)(addr);
						temp = temp;
					}
					if ( (resetAddress > userCodeVectorTable) &&
						 (resetAddress < lastFirmwareAddress) ){
						const uint32_t cmdAtResetVector = *( (uint32_t*) resetAddress );
						if (cmdAtResetVector != 0xFFFFFFFF){
							setDebugCode(8);
							regs->common.pc = (uint32_t)resetAddress;
							regs->common.lr = regs->common.pc;
							regs->common.sp = stackPointer;
							regs->xpsr = (regs->common.pc & 0x01) << 24;
							DEMCR |= DEMCR_MON_STEP;
							DEMCR &= ~DEMCR_MON_PEND;
							allowUsbTransfer = true;
							return;
							gHaltState = ResetHaltState::Running;
						}
					}

				} else {
					exitFromInterrupt = true;
					DEMCR |= DEMCR_MON_STEP;
					DEMCR &= ~DEMCR_MON_PEND;
					return;

					setDebugCode(0x0F);
					while(true){};
				}
			}
			break;

			default:
				while(true){};
			break;

			}
		} else {
			asm("wfi");
		}
	} while(ResetHaltState::Running != gHaltState);


	if (gCmdQueue.isEmpty()){
		// Don't reenter into debug handler
		DEMCR &= ~DEMCR_MON_PEND;
	}

	setDebugCode(7);
}


extern "C" void setDebugCode(const uint32_t code){
	register uint32_t temp = GPIOA_ODR;
	temp &= ~0x0F;
	temp |= (code & 0x0F);
	//GPIOA_ODR = temp;
}


