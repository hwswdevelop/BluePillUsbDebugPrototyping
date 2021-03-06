/*
 * SystemInit.cpp
 *
 *  Created on: 1 мар. 2021 г.
 *      Author: Evgeny
 */
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f1/st_usbfs.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/mpu.h>
#include <libopencm3/cm3/systick.h>
#include "Usb.h"
#include "Debug.h"

extern "C" void systemInit(){
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_USB);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO11);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO12);
	//gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO15);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
	//gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, ( GPIO0 | GPIO1 | GPIO2 | GPIO3 ) );

	AFIO_MAPR = AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON;
	//GPIOA_BRR = GPIO15;
	GPIOC_BSRR = GPIO13;

	// Configure interrupt handlers priority
	for(int i = 0; i < NVIC_IRQ_COUNT; i++){
		nvic_disable_irq(i);
		nvic_set_priority(i, 0xF0);
	}

	systick_set_frequency(1000, 72000000);
	nvic_set_priority( NVIC_SYSTICK_IRQ, 0x10);
	nvic_enable_irq(NVIC_SYSTICK_IRQ);
	systick_interrupt_enable();
	systick_counter_enable();
	nvic_set_priority( NVIC_SV_CALL_IRQ, 0x20);
	nvic_enable_irq(NVIC_SV_CALL_IRQ);

	nvic_set_priority(-4, 0x20); // DebugHandler ????
	nvic_enable_irq(-4);

	// Enable debugging
	DHCSR = DHCSR_DBGKEY | DHCSR_C_DEBUGEN;
	// Enable debugging
	DEMCR = DEMCR_MON_EN;
	// Count of breakpoints
	const uint32_t totalBpCount = (FPB->FP_CTRL & FP_CTRL_NUM_CODE_MASK) >> FP_CTRL_NUM_CODE_SHIFT;
	for( uint32_t bpNum = 0; bpNum < totalBpCount; bpNum++ ){
		volatile uint32_t* bp = &FPB->FP_COMP[bpNum];
		//*bp  = 0;
	}
	usbdInit();
}

volatile uint32_t timeFromSystemStartedMs = 0;

extern "C" void SysTick_Handler(){
	nvic_clear_pending_irq(NVIC_SYSTICK_IRQ);
	timeFromSystemStartedMs++;
}

