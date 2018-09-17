/*
	co2_module.h - Header for the CO2 Module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for the MH-Z14 CO2 sensor.
			
	2017/03/13, Maya Posch
*/


#ifndef CO2_MODULE_H
#define CO2_MODULE_H


#include "base_module.h"


class CO2Module {
	static Timer timer;
	static uint8_t readCmd[9];
	static uint8 eventLevel;
	static uint8 eventCountDown;
	static uint8 eventCountUp;
	
	static void onSerialReceived(Stream &stream, char arrivedChar, unsigned short availableCharsCount);
	
public:
	static bool initialize();
	static bool start();
	static bool shutdown();
	static void readCO2();
	static void config(String cmd);
};


#endif
