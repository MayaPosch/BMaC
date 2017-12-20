/*
	bme280_module.h - Header for the BME280 module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for ESP8266 BME280 functionality.
			
	2017/05/10, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef BME280_MODULE_H
#define BME280_MODULE_H


#include "ota_core.h"

#include <Libraries/BME280/BME280.h>


// I2C pins.
//#define SCL_PIN 5 // SCL pin: GPIO5 ('D1' on NodeMCU)
//#define SDA_PIN 4 // SDA pin: GPIO4 ('D2' on NodeMCU)


class BME280Module {
	static BME280* bme280;
	//static int sclPin;
	//static int sdaPin;
	static Timer timer;
	
public:
	static bool init();
	static bool shutdown();
	static void config(String cmd);
	static void readSensor();
};


#endif
