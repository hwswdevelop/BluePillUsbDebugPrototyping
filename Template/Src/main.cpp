/*
 * main.cpp
 *
 *  Created on: Mar 6, 2021
 *      Author: Evgeny
 */


#include <stdio.h>
#include <delay.h>
#include <stdint.h>
#include <gpio.h>


void setup(){
	printf("setup\n");
	PC13.mode(GpioMode::OutputPushPull, GpioOutputSpeed::Max50Mhz);
	PA0.mode(GpioMode::OutputPushPull, GpioOutputSpeed::Max50Mhz);
}

void loop(){

	uint32_t timeDiffMs = 0;
	uint32_t stopTimeMs = 0;
	uint32_t startTimeMs = millis();
	PC13.reset();
	for(uint32_t i = 0; i < 1000000; i++){
		PA0.set();
		PA0.invert();
		PA0.set();
		PA0.reset();
		PA0.invert();
		PA0.invert();
	}
	PC13.set();
	stopTimeMs = millis();
	timeDiffMs = stopTimeMs - startTimeMs;
	printf("Total time: %lu ms\n", timeDiffMs);

	uint32_t timeMs = millis();
	uint32_t timeS = timeMs / 1000;
	uint32_t timeM = timeS / 60;
	uint32_t timeH = timeM / 60;
	uint32_t day = timeH / 24;
	timeMs %= 1000;
	timeS %= 60;
	timeM %= 60;
	timeH %= 24;
	printf("Time from start day %03lu time %02lu:%02lu:%02lu.%03lu\n",  day, timeH, timeM, timeS, timeMs);

	delay(1000);

}

