/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f1/st_usbfs.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/mpu.h>
#include <libopencm3/stm32/usart.h>


static inline uint32_t millis()
{
	static volatile uint32_t timeMs;
	uint32_t address = (uint32_t)(&timeMs);
    asm volatile ("mov r0, %0; movs r3, #2; svc 0;" : "=r" (address));
    return timeMs;
}

void delay(uint32_t waitTimeMs){
	// Wait outside system
	volatile const uint32_t currentTime = millis();
	volatile const uint32_t endTime = currentTime + waitTimeMs;
	while(endTime > millis());
}

void enableDisabeInterrupt(uint32_t irq, bool enable){
	asm volatile ("movs r3, #1; svc 0;");
}

void test1(){
	for(int i =0; i < 50; i++){
		delay(100);
		uint32_t* const gpioc = (uint32_t*)0x4001100c;
		*gpioc ^= (1 << 13);
	}
}

void test2(){
	volatile uint32_t prevTime = millis();
	const uint32_t endTime = prevTime + 500000;
	volatile uint32_t currentTime = prevTime;
	do {
		currentTime = millis();
		uint32_t timeDiff = currentTime - prevTime;
		if (timeDiff > 500UL){
			prevTime = currentTime;
			uint32_t* const gpioc = (uint32_t*)0x4001100c;
			(*gpioc) ^= (1 << 13);
		}
	} while(currentTime < endTime);
}

void USART1_IRQHandler(){
	USART1_SR &= ~USART_SR_TC;
	nvic_clear_pending_irq(37);
	uint32_t* const gpioc = (uint32_t*)0x4001100c;
	//*gpioc ^= (1 << 13);
	usart_send(USART1, 0x55);
}

int main(void)
{

	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO9);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO10);
	enableDisabeInterrupt(37, false);
	rcc_periph_reset_pulse(RCC_USART1);
	rcc_periph_clock_enable(RCC_USART1);
	usart_set_baudrate(USART1, 115200);
	USART1_SR &= ~USART_SR_TC;
	USART1_CR1 |= USART_CR1_UE;
	usart_enable_tx_complete_interrupt(USART1);
	USART1_CR1 |= USART_CR1_TE;
	enableDisabeInterrupt(37, true);
	usart_send(USART1, 0x55);

	while(true){
		test1();
		test2();
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}
}