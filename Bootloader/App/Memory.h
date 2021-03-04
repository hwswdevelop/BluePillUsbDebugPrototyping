/*
 * Memory.h
 *
 *  Created on: 1 мар. 2021 г.
 *      Author: Evgeny
 */

#pragma once

uint32_t checkMemRegionAccess(uint32_t begin, uint32_t end);
void dbg_memcpy(uint8_t* const dst, const uint8_t* const src, const uint32_t size, const uint32_t align);
