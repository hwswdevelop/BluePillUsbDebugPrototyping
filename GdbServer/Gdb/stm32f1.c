/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file implements STM32 target specific functions for detecting
 * the device, providing the XML memory map and Flash memory programming.
 *
 * Refereces:
 * ST doc - RM0008
 *   Reference manual - STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx
 *   and STM32F107xx advanced ARM-based 32-bit MCUs
 * ST doc - RM0091
 *   Reference manual - STM32F0x1/STM32F0x2/STM32F0x8
 *   advanced ARM®-based 32-bit MCUs
 * ST doc - RM0360
 *   Reference manual - STM32F030x4/x6/x8/xC and STM32F070x6/xB
 * ST doc - PM0075
 *   Programming manual - STM32F10xxx Flash memory microcontrollers
 */

#include "general.h"
#include "target.h"
#include "target_internal.h"


static int stm32f1_flash_erase(struct target_flash *f,
                               target_addr addr, size_t len);
static int stm32f1_flash_write(struct target_flash *f,
                               target_addr dest, const void *src, size_t len);

/* Flash Program ad Erase Controller Register Map */
#define FPEC_BASE	0x40022000
#define FLASH_ACR	(FPEC_BASE+0x00)
#define FLASH_KEYR	(FPEC_BASE+0x04)
#define FLASH_OPTKEYR	(FPEC_BASE+0x08)
#define FLASH_SR	(FPEC_BASE+0x0C)
#define FLASH_CR	(FPEC_BASE+0x10)
#define FLASH_AR	(FPEC_BASE+0x14)
#define FLASH_OBR	(FPEC_BASE+0x1C)
#define FLASH_WRPR	(FPEC_BASE+0x20)

#define FLASH_BANK2_OFFSET 0x40
#define FLASH_BANK_SPLIT   0x08080000

#define FLASH_CR_OBL_LAUNCH (1<<13)
#define FLASH_CR_OPTWRE	(1 << 9)
#define FLASH_CR_STRT	(1 << 6)
#define FLASH_CR_OPTER	(1 << 5)
#define FLASH_CR_OPTPG	(1 << 4)
#define FLASH_CR_MER	(1 << 2)
#define FLASH_CR_PER	(1 << 1)
#define FLASH_CR_PG		(1 << 0)

#define FLASH_OBR_RDPRT (1 << 1)

#define FLASH_SR_BSY	(1 << 0)

#define FLASH_OBP_RDP 0x1FFFF800
#define FLASH_OBP_RDP_KEY 0x5aa5
#define FLASH_OBP_RDP_KEY_F3 0x55AA

#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

#define SR_ERROR_MASK	0x14
#define SR_EOP		0x20

#define DBGMCU_IDCODE	0xE0042000
#define DBGMCU_IDCODE_F0	0x40015800

#define FLASHSIZE     0x1FFFF7E0
#define FLASHSIZE_F0  0x1FFFF7CC


void stm32_flash_unlock(target * t, uint32_t address, size_t size);
void stm32_flash_erase(target* t, uint32_t address, size_t size);
void stm32_flash_wrtie(target * t, uint32_t address, uint8_t * data, size_t size);
void stm32_flash_done(target * t);
int stm32f1_flash_done(struct target_flash* f);

static void stm32f1_add_flash(target *t,
                              uint32_t addr, size_t length, size_t erasesize)
{
	struct target_flash *f = calloc(1, sizeof(*f));
	if (!f) {			/* calloc failed: heap exhaustion */
		DEBUG_WARN("calloc: failed in %s\n", __func__);
		return;
	}

	f->start = addr;
	f->length = length;
	f->blocksize = erasesize;
	f->erase = stm32f1_flash_erase;
	f->write = stm32f1_flash_write;
	f->done = stm32f1_flash_done;
	f->buf_size = erasesize;
	f->erased = 0xff;
	target_add_flash(t, f);
}


bool stm32f1_probe(target *t)
{
	uint16_t stored_idcode = t->idcode;
	target_add_ram(t, 0x20000000, 0x5000);
	target_add_ram(t, 0x40001000, 0x023400);
	stm32f1_add_flash(t, 0x8000000, 0x20000, 0x400);
	return true;
}

static void stm32f1_flash_unlock(target *t, uint32_t bank_offset)
{
	stm32_flash_unlock(t, bank_offset, 4);
}

static int stm32f1_flash_erase(struct target_flash *f,
                               target_addr addr, size_t len)
{
	target *t = f->t;
	stm32_flash_erase(t, addr, len);
	return 0;
}

static int stm32f1_flash_write(struct target_flash *f,
                               target_addr dest, const uint8_t *src, size_t len)
{
	target *t = f->t;
	stm32_flash_wrtie(t, dest, src, len);
	return 0;
}

int stm32f1_flash_done(struct target_flash* f) {
	target* t = f->t;
	stm32_flash_done(t);
	return 0;
}