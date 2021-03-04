/*
 * Debug.h
 *
 *  Created on: 7 февр. 2021 г.
 *      Author: Evgeny
 */

#pragma once

#include <cstdint>

#define NUMOFBKPTS 			(8)
#define NUMOFWPTS 			(3)

typedef struct {
  uint32_t FP_CTRL;
  uint32_t FP_REMAP;
  uint32_t FP_COMP[NUMOFBKPTS];
} FPBUnit, *pFPBUnit;

typedef struct  {
	uint32_t DWT_COMP;
	uint32_t DWT_MASK;
	uint32_t DWT_FUNCTION;
} DWComp, *pDWComp;

typedef struct {
	uint32_t DWT_CTRL;
	uint32_t DWT_CYCCNT;
	uint32_t DWT_CPICNT;
	uint32_t DWT_EXCCNT;
	uint32_t DWT_SLEEPCNT;
	uint32_t DWT_LSUCNT;
	uint32_t DWT_FOLDCNT;
	uint32_t DWT_PCSR;
	DWComp   CMP[NUMOFWPTS];
} DWUnit, *pDWUnit;


#define DWT_CTRL_NUMCOMP_SHIFT	(28)
#define DWT_CTRL_NUMCOMP_MASK	(0x0F << 28)
#define DWR_CTRL_NUMCOMP DWT_CTRL

/* DW Control Register */
#define DW						( (pDWUnit)  0xE0001000 )
#define FPB						( (pFPBUnit) 0xE0002000 )
/* Debug Halting Control and Status Register */
#define DHCSR					( *(uint32_t*)0xE000EDF0 )
/* Debug Core Register Selector Register */
#define DCRSR					( *(uint32_t*)0xE000EDF4 )
/* Debug Core Register Data Register */
#define DCRDR					( *(uint32_t*)0xE000EDF8 )
/* Debug Exception and Monitor Control Register */
#define DEMCR					( *(uint32_t*)0xE000EDFC )

#define FP_CTRL_ENABLE			(1 << 0)
#define FP_CTRL_KEY				(1 << 1)
#define FP_CTRL_NUM_CODE_SHIFT	(4)
#define FP_CTRL_NUM_CODE_MASK	(0x0F << FP_CTRL_NUM_CODE_SHIFT)
#define FP_CTRL_NUM_LIT_SHIFT	(8)
#define FP_CTRL_NUM_LIT_MASK	(0x0F << FP_CTRL_NUM_LIT_SHIFT)
#define FP_CTRL_NUM_CODEH_SHIFT	(12)
#define FP_CTRL_NUM_CODEH_MASK	(0x07 << FP_CTRL_NUM_CODEH_SHIFT)

#define FP_REMAP_REMAP_SHIFT	(5)
#define FP_REMAP_REMAP_MASK		(0xFFFFFF << FP_REMAP_REMAP_SHIFT)
#define FP_REMAP_RMPSPT			(1 << 29)

/* Debug Halting Control and Status Register */
#define DHCSR_C_DEBUGEN			(1 << 0)
#define DHCSR_C_HALT			(1 << 1)
#define DHCSR_C_STEP			(1 << 2)
#define DHCSR_C_MASKINTS		(1 << 3)
#define DHCSR_C_SNAPSTALL		(1 << 5)
#define DHCSR_S_REGRDY			(1 << 16)
#define DHCSR_S_HALT			(1 << 17)
#define DHCSR_S_SLEEP			(1 << 18)
#define DHCSR_S_LOCKUP			(1 << 19)
#define DHCSR_S_RETIRE_ST		(1 << 24)
#define DHCSR_S_RESET_ST		(1 << 25)
#define DHCSR_DBGKEY			(0xA05F << 16)

/* Debug Core Register Selector Register */
#define DCRSR_REGWnR			(1 << 16)
#define DCRSR_REGSEL_MASK		(0x001F << 0)
#define DCRSR_REGSEL_R0			(0x00 << 0)
#define DCRSR_REGSEL_R1			(0x01 << 0)
#define DCRSR_REGSEL_R2			(0x02 << 0)
#define DCRSR_REGSEL_R3			(0x03 << 0)
#define DCRSR_REGSEL_R4			(0x04 << 0)
#define DCRSR_REGSEL_R5			(0x05 << 0)
#define DCRSR_REGSEL_R6			(0x06 << 0)
#define DCRSR_REGSEL_R7			(0x07 << 0)
#define DCRSR_REGSEL_R8			(0x08 << 0)
#define DCRSR_REGSEL_R9			(0x09 << 0)
#define DCRSR_REGSEL_R10		(0x0A << 0)
#define DCRSR_REGSEL_R11		(0x0B << 0)
#define DCRSR_REGSEL_R12		(0x0C << 0)
#define DCRSR_REGSEL_R13		(0x0D << 0)
#define DCRSR_REGSEL_R14		(0x0E << 0)
#define DHCSR_REGSEL_RET		(0x0F << 0)
#define DHCSR_REGSEL_MSP		(0x11 << 0)
#define DHCSR_REGSEL_PSP		(0x12 << 0)
#define DHCSR_REGSEL_RAZ_WI		(0x13 << 0)

/* Debug Exception and Monitor Control Register */
#define DEMCR_VC_CORERESET		(1 << 0)
#define DEMCR_VC_MMERR			(1 << 4)
#define DEMCR_VC_NOCPERR		(1 << 5)
#define DEMCR_VC_CHKERR			(1 << 6)
#define DEMCR_VC_STATERR		(1 << 7)
#define DEMCR_VC_BUSERR			(1 << 8)
#define DEMCR_VC_INTERR			(1 << 9)
#define DEMCR_VC_VC_HARDERR		(1 << 10)
#define DEMCR_MON_EN			(1 << 16)
#define DEMCR_MON_PEND			(1 << 17)
#define DEMCR_MON_STEP			(1 << 18)
#define DEMCR_MON_REQ			(1 << 19)
#define DEMCR_TRCENA			(1 << 24)


struct ArmCommonRegs {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
};

struct ArmRegisters {
	union {
		ArmCommonRegs common;
		uint32_t r[16];
	};
	uint32_t xpsr;
};


#include "DbgCommands.h"

extern "C" void setDebugCode(const uint32_t code);
extern "C" ResetHaltState debugGetHaltState();
extern "C" bool debugHaltRequest();
extern "C" bool debugResume(bool step);
extern "C" bool debugStartFirmware();
extern "C" bool debugStartLocalLoop();
extern "C" bool debugStartEraseLoop();
extern "C" volatile ArmRegisters* const debugGetRegs();

extern "C" __attribute__((naked)) void debug_handler();
extern "C" __attribute__((naked)) void sysIrqHandler(uint32_t handler);
