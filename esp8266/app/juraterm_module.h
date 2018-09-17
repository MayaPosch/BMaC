/*
	jura_module.h - Header for the Jura Module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for Jura serial protocol.
			
	2017/03/13, Maya Posch
*/


#ifndef JURATERM_MODULE_H
#define JURATERM_MODULE_H


#include "base_module.h"


class JuraTermModule {
	static String mqttTxBuffer;
	
	static bool toCoffeemaker(String cmd);
	static void onSerialReceived(Stream &stream, char arrivedChar, unsigned short availableCharsCount);
	
public:
	static bool initialize();
	static bool start();
	static bool shutdown();
	static void commandCallback(String message);
};


#endif
