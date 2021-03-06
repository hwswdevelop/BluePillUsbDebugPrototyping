/*
 * delay.cpp
 *
 *  Created on: Mar 6, 2021
 *      Author: Evgeny
 */
#include "delay.h"

void delay(uint32_t waitTimeMs){
	// Wait outside system
	volatile const uint32_t currentTime = millis();
	volatile const uint32_t endTime = currentTime + waitTimeMs;
	while(endTime > millis());
}

void enableDisabeInterrupt(uint32_t irq, bool enable){
	asm volatile ("movs r3, #1; svc 0;");
}

