/*
 * delay.h
 *
 *  Created on: Mar 6, 2021
 *      Author: Evgeny
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

volatile uint32_t millis();
void delay(uint32_t waitTimeMs);

#ifdef __cplusplus
}
#endif
