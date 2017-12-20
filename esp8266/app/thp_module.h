/*
	thp_module.h - Header for the Temperature/Humidity/Pressure module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for this functionality.
			
	2017/05/10, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef THP_MODULE_H
#define THP_MODULE_H


#include "ota_core.h"

#include <Libraries/DHT/DHT.h>



class THPModule {
	//static DHT* dht;
	//static BME280* bme280;
	//static int dhtPin;
	//static Timer timer;
	
public:
	static bool init();
	static bool shutdown();
	static void config(String cmd);
};


#endif
