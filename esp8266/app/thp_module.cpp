/*
	thp_module.cpp - THPModule class implementation.
	
	Revision 0
	
	Features:
			- Defines the THP module class needed for ESP8266 functionality.
			
	2017/05/10, Maya Posch <posch@synyx.de>
*/


#include "thp_module.h"

#include "dht_module.h"
#include "bme280_module.h"


// Static initialisations.
//DHT* THPModule::dht = 0;
//BME280* THPModule
//Timer THPModule::timer;


// --- INIT ---
bool THPModule::init() {
	// Directly start the BME280 module here for now.
	// TODO: make configurable.
	BME280Module::init();
	
	// Ensure we got a sensor class instance.
	//if (!dht) { dht = new DHT(dhtPin); }
	
	// Wait for sensor startup, then start DHT logging.
	//WDT.enable(false); // disable watchdog
	//delay(1000);
	//dht->begin();
	
	// Create timer.
	//timer.initializeMs(2000, THPModule::readDHT).start();
	//timer.initializeMs(2000, THPModule::readDHT).start();
	
	return true;
}


// --- SHUTDOWN ---
bool THPModule::shutdown() {
	// TODO: make configurable.
	BME280Module::shutdown();
	
	//dhtTimer.stop();
	
	// There's no 'end' or 'stop' method in the DHT class. Just delete it.
	//delete dht;
	//dht = 0;
	
	return true;
}


// --- CONFIG ---
// Processes any configuration commands for this module.
// Commands are provided in the following format:
// <command string>=<payload string>
void THPModule::config(String cmd) {
	Vector<String> output;
	int numToken = splitString(cmd, '=', output);
	if (output[0] == "set_pin" && numToken > 1) {
		//dhtPin = output[1].toInt();
		
		// TODO: send to the active module.
	}
}
