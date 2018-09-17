/*
	dht_module.h - Header for the DHT module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for ESP8266 DHT functionality.
			
	2017/03/13, Maya Posch
*/


#pragma once
#ifndef DHT_MODULE_H
#define DHT_MODULE_H


#include "ota_core.h"

#include <Libraries/DHTesp/DHTesp.h>


#define DHT_PIN 5 // DHT sensor: GPIO5 ('D1' on NodeMCU)


class DHTModule {
	static DHTesp* dht;
	static int dhtPin;
	static Timer dhtTimer;
	
public:
	static bool init();
	static bool shutdown();
	static void config(String cmd);
	static void readDHT();
};


#endif
