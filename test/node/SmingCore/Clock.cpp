/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "Clock.h"
#include <chrono>

#define MAX_SAFE_DELAY 1000

unsigned long millis() {
	unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return now;
}

// TODO: implement.
unsigned long micros() {
	unsigned long now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return now;
}

// TODO: implement when required.
void delay(uint32_t milliseconds) {
	/* unsigned quotient = milliseconds / MAX_SAFE_DELAY;
	unsigned remainder = milliseconds % MAX_SAFE_DELAY;
	for(unsigned i = 0; i <= quotient; i++) {
		if(i == quotient) {
			os_delay_us(remainder * 1000);
		} else {
			os_delay_us(MAX_SAFE_DELAY * 1000);
		}

		system_soft_wdt_feed();
	} */
}

// TODO: implement when required.
void delayMicroseconds(uint32_t time) {
	//os_delay_us(time);
}
