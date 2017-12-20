/*
	bme280_module.cpp - BME280Module class implementation.
	
	Revision 0
	
	Features:
			- Defines the BME280 module class needed for ESP8266 BME280 functionality.
			
	2017/05/10, Maya Posch <posch@synyx.de>
*/


#include "bme280_module.h"


// Static initialisations.
BME280* BME280Module::bme280 = 0; //(DHT_PIN);
//int BME280Module::sclPin = SCL_PIN; // default.
//int BME280Module::sdaPin = SDA_PIN; // default.
Timer BME280Module::timer;


// --- INIT ---
bool BME280Module::init() {
	// Start i2c comms.
	OtaCore::starti2c();
	
	/*// First perform a reset in case the slave device is stuck. For this we
	// pulse SCL 8 times.
	Wire.pins(sdaPin, sclPin);
	pinMode(sclPin, OUTPUT);
	for (int i = 0; i < 8; ++i) {
		digitalWrite(sclPin, HIGH);
		delayMicroseconds(3);
		digitalWrite(sclPin, LOW);
		delayMicroseconds(3);
	}
	  
	pinMode(sclPin, INPUT);
  
	// Next start the i2c bus.
	Wire.begin(); */
	
	// Ensure we got a sensor class instance.
	if (!bme280) { bme280 = new BME280(); }
	
	// Wait for sensor startup, then start logging.
	if (bme280->EnsureConnected()) {
		OtaCore::log(LOG_INFO, "Connected to BME280 sensor.");
		bme280->SoftReset(); // Ensure clean start.
		bme280->Initialize(); // Initialise and pull calibration data (?).
	}
	else {
		OtaCore::log(LOG_ERROR, "Not connected to BME280 sensor.");
		return false;
	}
	
	// Create timer.
	timer.initializeMs(2000, BME280Module::readSensor).start();
	
	return true;
}


// --- SHUTDOWN ---
bool BME280Module::shutdown() {
	timer.stop();
	
	// There's no 'end' or 'stop' method in the BME280 class. Just delete it.
	delete bme280;
	bme280 = 0;
	
	return true;
}


// --- CONFIG ---
// Processes any configuration commands for this module.
// Commands are provided in the following format:
// <command string>=<payload string>
void BME280Module::config(String cmd) {
	Vector<String> output;
	int numToken = splitString(cmd, '=', output);
	if (output[0] == "set_pin" && numToken > 1) {
		//dhtPin = output[1].toInt();
	}
}


// --- READ SENSOR ---
void BME280Module::readSensor() {
	float t, h, p;
	if (bme280->IsConnected) {
		t = bme280->GetTemperature();
		h = bme280->GetHumidity();
		p = bme280->GetPressure();
		
		// Publish via MQTT.
		OtaCore::publish("nsa/temperature", OtaCore::getLocation() + ";" + t);
		OtaCore::publish("nsa/humidity", OtaCore::getLocation() + ";" + h);
		OtaCore::publish("nsa/pressure", OtaCore::getLocation() + ";" + p);
	}
	else {
		OtaCore::log(LOG_ERROR, "Disconnected from BME280 sensor.");
	}
}
