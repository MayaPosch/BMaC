/*
	main.cpp - Main header of the OTA Unified firmware.
	
	Revision 0
	
	Features:
			- 
			
	2017/03/13, Maya Posch
*/


#include "ota_core.h"


void onInit() {
	// 
}


void init() {
	// Initialise the OTA Core class.
	OtaCore::init(onInit);
}
