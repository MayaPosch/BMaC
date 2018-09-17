/*
	thp_module.cpp - THPModule class implementation.
	
	Revision 0
	
	Features:
			- Defines the THP module class needed for ESP8266 functionality.
			
	2017/05/10, Maya Posch
*/


#include "thp_module.h"

#include "dht_module.h"
#include "bme280_module.h"


// Static initialisations.
//bool THPModule::thp_init = THPModule::thp_initialize();


// --- THP INIT ---
bool THPModule::initialize() {
	BaseModule::registerModule(MOD_IDX_TEMPERATURE_HUMIDITY, THPModule::start, THPModule::shutdown);
}


// --- START ---
bool THPModule::start() {
	// Directly start the BME280 module here for now.
	// TODO: make configurable.
	BME280Module::init();
	
	return true;
}


// --- SHUTDOWN ---
bool THPModule::shutdown() {
	// TODO: make configurable.
	BME280Module::shutdown();
	
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
