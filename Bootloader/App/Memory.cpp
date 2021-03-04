/*
 * MemoryRegions.cpp
 *
 *  Created on: 1 мар. 2021 г.
 *      Author: Evgeny
 */


#include <stdint.h>
#include "Debug.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/mpu.h>

struct MemoryRegion {
	uint32_t begin;
	uint32_t end;
	uint32_t attributes;
};

const MemoryRegion memoryAccessRegions[] = {
		{ // Flash
				0x08000000,
				0x08020000,
				0x03,
		},
		{ // SRAM
				0x20000000,
				0x20010000,
				0x03,
		},
		{ // Cortex M3 peripherlas
				0xE0000000,
				0xE0100000,
				0x03,
		},
		{ // TIM2, TIM3, TIM4
				0x40000000,
				0x40000C00,
				0x03,
		},
		{ // RTC, WWDG, IWDG
				0x40002800,
				0x40003400,
				0x03,
		},
		{ // SPI2
				0x40003800,
				0x40003C00,
				0x03,
		},
		{ // USART2, USART3
				0x40004400,
				0x40004C00,
				0x03,
		},
		{ // I2C1, I2C2
				0x40005400,
				0x40005C00,
				0x03,
		},
		{ // bcCAN
				0x40006400,
				0x40006800,
				0x03,
		},
		{ // BKP, PWR
				0x40006C00,
				0x40007400,
				0x03,
		},
		{ // AFIO, EXTI, PORTA, PORTB, PORTC, PORTD, PORTE
				0x40010000,
				0x40011C00,
				0x03,
		},
		{ // ADC1, ADC2, TIM1, SPI1
				0x40012400,
				0x40013400,
				0x03,
		},
		{ // USART1
				0x40013800,
				0x40013C00,
				0x03,
		},
		{ // DMA
				0x40020000,
				0x40020400,
				0x03,
		},
		{ // RCC
				0x40021000,
				0x40021400,
				0x03,
		},
		{ // Flash
				0x40022000,
				0x40022400,
				0x03,
		},
		{ // CRC
				0x40023000,
				0x40023400,
				0x03,
		},
};

uint32_t checkMemRegionAccess(uint32_t begin, uint32_t end){
	const uint32_t regionCount = sizeof(memoryAccessRegions) / sizeof(struct MemoryRegion);
	for( uint32_t memRegIndex = 0; memRegIndex < regionCount; memRegIndex++ ){
		const MemoryRegion* region = &memoryAccessRegions[memRegIndex];
		if ( ( region->begin <= begin ) &&
		     ( end <= region->end ) ){
			return region->attributes;
		}
	}
	return 0;
}


static inline  uint32_t __get_FAULTMASK(void)
{
  uint32_t result;
  asm volatile ("MRS %0, faultmask" : "=r" (result) );
  return(result);
}

static inline void __set_FAULTMASK(uint32_t faultMask)
{
  asm volatile ("MSR faultmask, %0" : : "r" (faultMask) : "memory");
}

static inline void __disable_fault_irq(void)
{
  asm volatile ("cpsid f" : : : "memory");
}

static inline void __enable_fault_irq(void)
{
  asm volatile ("cpsie f" : : : "memory");
}

void dbg_memcpy(uint8_t* const dst, const uint8_t* const src, const uint32_t size, const uint32_t align) {

	static constexpr const uint32_t SCB_CFSR_BUSFAULTSR_Pos = 8;
	static constexpr const uint32_t SCB_CCR_BFHFNMIGN_Pos = 8;
	static constexpr const uint32_t BFARVALID_MASK = (0x80 << SCB_CFSR_BUSFAULTSR_Pos);
	static constexpr const uint32_t SCB_CCR_BFHFNMIGN_Msk  (1UL << SCB_CCR_BFHFNMIGN_Pos);
	uint32_t copySize = size;

	SCB_CFSR |= BFARVALID_MASK;
	uint32_t mask = __get_FAULTMASK();
	__disable_fault_irq();
	SCB_CCR |= SCB_CCR_BFHFNMIGN_Msk;


	if (align == 4) {
		uint32_t* cdst = (uint32_t*)dst;
		uint32_t* csrc = (uint32_t*)src;
		while(copySize){
			*cdst++ = *csrc++;
			copySize -= sizeof(uint32_t);
		}
	} else if (align == 2) {
		uint16_t* cdst = (uint16_t*)dst;
		uint16_t* csrc = (uint16_t*)src;
		while(copySize){
			*cdst++ = *csrc++;
			copySize -= sizeof(uint16_t);
		}
	} else {
		uint8_t* cdst = (uint8_t*)dst;
		uint8_t* csrc = (uint8_t*)src;
		while(copySize){
			*cdst++ = *csrc++;
			copySize -= sizeof(uint8_t);
		}
	}

	SCB_CCR &= ~SCB_CCR_BFHFNMIGN_Msk;
	__set_FAULTMASK(mask);
}
