/*
	dht_module.cpp - DHTModule class implementation.
	
	Revision 0
	
	Features:
			- Defines the DHT module class needed for ESP8266 DHT functionality.
			
	2017/03/13, Maya Posch
*/


#include "dht_module.h"


// Static initialisations.
DHTesp* DHTModule::dht = 0; //(DHT_PIN);
int DHTModule::dhtPin = DHT_PIN; // default.
Timer DHTModule::dhtTimer;


// --- INIT ---
bool DHTModule::init() {
	// Register pins.
	if (!OtaCore::claimPin(dhtPin)) { return false; }
	
	// Ensure we got a sensor class instance.
	if (!dht) { dht = new DHTesp(); dht->setup(dhtPin, DHTesp::DHT22); }
	
	// Wait for sensor startup, then start DHT logging.
	/* WDT.enable(false); // disable watchdog
	delay(1000);
	dht->begin(); */
	
	// Create timer.
	dhtTimer.initializeMs(2000, DHTModule::readDHT).start();
	
	return true;
}


// --- SHUTDOWN ---
bool DHTModule::shutdown() {
	dhtTimer.stop();
	
	// Release pins.
	if (!OtaCore::releasePin((ESP8266_pins) dhtPin)) { return false; }
	
	// There's no 'end' or 'stop' method in the DHT class. Just delete it.
	delete dht;
	dht = 0;
	
	return true;
}


// --- CONFIG ---
// Processes any configuration commands for this module.
// Commands are provided in the following format:
// <command string>=<payload string>
void DHTModule::config(String cmd) {
	Vector<String> output;
	int numToken = splitString(cmd, '=', output);
	if (output[0] == "set_pin" && numToken > 1) {
		dhtPin = output[1].toInt();
	}
}


// --- READ DHT ---
void DHTModule::readDHT() {
	TempAndHumidity th;
	th = dht->getTempAndHumidity();
	//if (dht->readTempAndHumidity(th)) {
		
	// Publish via MQTT.
	OtaCore::publish("nsa/temperature", OtaCore::getLocation() + ";" + th.temperature);
	OtaCore::publish("nsa/humidity", OtaCore::getLocation() + ";" + th.humidity);
	// (th.humid - 17.0)); // FIXME: Subtract 17% (hack for wrong resistor on sensors).
		
	/* }
	else {
		String err = "Failed to read from DHT: " + dht->getLastError();
		OtaCore::log(LOG_ERROR, err);
	} */
}
