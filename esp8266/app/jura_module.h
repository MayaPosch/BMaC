/*
	jura_module.h - Header for the Jura Module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for Jura serial protocol.
			
	2017/03/13, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef JURA_MODULE_H
#define JURA_MODULE_H


#include "ota_core.h"


class JuraModule {
	//static String mqttTxBuffer;
	static Timer timer;
	
	static bool toCoffeemaker(String cmd);
	static void readStatistics();
	//static void onSerialReceived(Stream &stream, char arrivedChar, unsigned short availableCharsCount);
	
public:
	static bool init();
	static bool shutdown();
};


#endif
