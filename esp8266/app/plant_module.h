/*
	plant_module.h - Header for the Plant Module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for plant monitoring and watering functionality.
			
	2018/04/21, Maya Posch
*/


#ifndef PLANT_MODULE_H
#define PLANT_MODULE_H

#include "base_module.h"

#include <Libraries/APA102/apa102.h>


#define PLANT_GPIO_PIN 5	// GPIO 5 (D1) by default.
#define NUM_APA102 1		// Number of connected LEDs.


class PlantModule {
	static int pin;
	static Timer timer;
	static uint16 humidityTrigger;
	static String publishTopic;
	static HttpServer server;
	static APA102* LED;
	
	static void onRequest(HttpRequest& request, HttpResponse& response);
	
public:
	static bool initialize();
	static bool start();
	static bool shutdown();
	static void readSensor();
	static void commandCallback(String message);
};


#endif
