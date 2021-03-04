/*
 * LoLevelDebug.s
 *
 *  Created on: 10 февр. 2021 г.
 *      Author: Evgeny
 */
  .syntax unified
  .cpu cortex-m3
  .thumb

.data
.section .data.context_data, "aw", %progbits
.global context_data
.align 4
context_data:
context_data_r0:
.space 4
context_data_r1:
.space 4
context_data_r2:
.space 4
context_data_r3:
.space 4
context_data_r4:
.space 4
context_data_r5:
.space 4
context_data_r6:
.space 4
context_data_r7:
.space 4
context_data_r8:
.space 4
context_data_r9:
.space 4
context_data_r10:
.space 4
context_data_r11:
.space 4
context_data_r12:
.space 4
context_data_r13:
context_data_sp:
.space 4
context_data_r14:
context_data_lr:
.space 4
context_data_r15:
context_data_pc:
.space 4
context_data_xpsr:
.space 4
context_data_msp:
.space 4
context_data_psp:
.space 4


// /* Register number tables */
//static const uint32_t regnum_cortex_m[] = {
//	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,	/* standard r0-r15 */
//	0x10,	/* xpsr */
//	0x11,	/* msp */
//	0x12,	/* psp */
//	0x14	/* special */
//};

// R13=SP, R14=LR, R15=PC
//**************
// STACK FRAME
// SP + 1C: xPSR
// SP + 18: PC
// SP + 14: LR
// SP + 10: R12
// SP + 0C: R3
// SP + 08: R2
// SP + 04: R1
// SP + 00: R0 <== PSP
//**************
.text
.section .text.debug_handler, "ax", %progbits
.global debug_handler
.global DebugMon_Handler
.thumb
.thumb_func
.align 4
DebugMon_Handler:
    ldr r12, = context_data_r4  // R12 <= &CONTEXT_R4
    stmia r12!, {r4-r11}        // { CONTEXT_R4 - CONTEXT_R11 } <= { R4 - R11 }
    tst lr, #4
    ite eq
    //mrs r4, PSP					// R4 <= PSP
    mrseq r4, MSP               // R4 <= MSP
    mrsne r4, PSP               // R4 <= PSP
    isb                         // Instruction Barier
    // { [...], [R4-R11], [...] }
    ldmia r4, {r0-r3,r5-r8}     // {R0-R4} = {STACK_R0-STACK_R4}, R5 = STACK_R12, R6=STACK_LR, R7=STACK_PC, R8=STACK_xPSR
    stmia r12!, {r5}            // CONTEXT_R12 = STACK_R12
    add r4, r4, 0x20			// PROCESS STACK POINTER WITHOUT STACKFRAME
    stmia r12!, {r4}            // CONTEXT_R13 = SP
    stmia r12!, {r6,r7,r8}      // CONTEXT_R14 = LR, CONTEXT_R15 = PC, CONTEXT_xPSR = xPSR
    sub r11, r12, #68           // R11 <= CONTEXT_R0
    stmia r11, {r0-r3}          // { CONTEXT_R0-CONTEXT_R4 } <= { R0-R4}
    mov r0, r11					// R0 = context pointer
    push {lr}
    bl DebugHandler
    pop {lr}
    ldr r12, = context_data_r0  // R12 = &CONTEXT_R0
    ldr r11, [r12, #52]         // R11 = {CONTEXT_SP}
    sub r11, r11, 0x20          // R11 = HANDLER_SP_CONTEXT
    tst lr, #4
    ite eq
    msreq MSP, r11              // MSP <= R11
    msrne PSP, r11              // PSP <= R11
    //msr PSP, r11              	// PSP is now equals to HANDLER_SP_CONTEXT
    isb                         // Instruction Barier
    ldmia r12!, {r0-r3}			// {R0-R3} <= {CONTEXT_R0-CONTEXT_R3}
    stmia r11!, {r0-r3}         // {STACK_R0-STACK_R3} <= {R0-R3}
    mov r0, r11                 // R0 - SP + 0x10
    ldmia r12!, {r4-r11}        // {R4-R11} = {CONTEXT_R4-CONTEXT_R11}
    ldmia r12!, {r1}			// R1 = CONTEXT_R12
    add r12, r12, #4			// Skip CONTEXT_SP
    ldmia r12!, {r2,r3}			// R2 = CONTEXT_LR, R3 = CONTEXT_PC
	stmia r0!, {r1-r3}          // {STACK_R12, STACK_LR, STACK_PC} <= { R1, R2, R3 }
	ldmia r12!, {r1}            // R1 = CONTEXT_xPSR
	stmia r0!, {r1}             // STACK_xPSR = CONTEXT_xPSR
	bx lr



.text
.section .text.AnyUserIRQHandler, "ax", %progbits
.global AnyUserIRQHandler
.thumb
.thumb_func
.align 4
AnyUserIRQHandler:
    mrs r0, IPSR
    isb
    lsl r1, r0, #2
	adds r1, r1, #0x008000000
	adds r1, r1, #0x4000
	ldr r1, [r1]
	ldr r3, =0x8000000
	cmp r1, r3
	it le
	bxle lr
	ldr r3, =0x8020000
	cmp r1, r3
	it ge
	bxge lr
	ldr r0, [r1]
	cmp r0, #0xFFFFFFFF
	it eq
	bxeq lr
	push {r0, lr}
	movs r0, 12
	bl setDebugCode
	pop {r0, lr}
	bx r1


// R13=SP, R14=LR, R15=PC
//**************
// STACK FRAME
// SP + 1C: xPSR
// SP + 18: PC
// SP + 14: LR
// SP + 10: R12
// SP + 0C: R3
// SP + 08: R2
// SP + 04: R1
// SP + 00: R0 <== PSP
//**************
.text
.section .text.USB_LP_CAN1_RX0_IRQHandler, "ax", %progbits
.global USB_LP_CAN1_RX0_IRQHandler
.global USBWakeUp_IRQHandler
.global USB_HP_CAN1_TX_IRQHandler
.thumb
.thumb_func
.align 4

USB_LP_CAN1_RX0_IRQHandler:
USBWakeUp_IRQHandler:
USB_HP_CAN1_TX_IRQHandler:
	push {lr}
	bl USB_IRQHandler
	pop {lr}
	bx lr


.global WWDG_IRQHandler
.global UsageFault_Handler
.global NMI_Handler
.global HardFault_Handler
.global MemManage_Handler
.global BusFault_Handler

WWDG_IRQHandler:
	ldr r0, =0xE000EDFC
	ldr r1, [r0]
	orr r1, r1, #((1 << 17) | (1 << 18));
	str r1, [r0]
	bx lr

UsageFault_Handler:
	ldr r0, =0xE000EDFC
	ldr r1, [r0]
	orr r1, r1, #((1 << 17) | (1 << 18));
	str r1, [r0]
	bx lr

NMI_Handler:
    ldr r0, =0xE000EDFC
	ldr r1, [r0]
	orr r1, r1, #((1 << 17) | (1 << 18));
	str r1, [r0]
	bx lr

.global setDebugCode
HardFault_Handler:
    push {lr}
    movs r0, #0x0F
    bl setDebugCode
    pop {lr}
    ldr r0, =0xE000EDFC
	ldr r1, [r0]
	orr r1, r1, #((1 << 17) | (1 << 18));
	str r1, [r0]
	bx lr

MemManage_Handler:
	ldr r0, =0xE000EDFC
	ldr r1, [r0]
	orr r1, r1, #((1 << 17) | (1 << 18));
	str r1, [r0]
	bx lr

BusFault_Handler:
	ldr r0, =0xE000EDFC
	ldr r1, [r0]
	orr r1, r1, #((1 << 17) | (1 << 18));
	str r1, [r0]
	bx lr

