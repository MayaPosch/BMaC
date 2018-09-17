/*
	co2_module.cpp - CO2 Module class implementation.
	
	Revision 0
	
	Features:
			- Defines the module for MH-Z14 CO2 sensor support.
			- Publishes read values via OtaCore::publish().
			
	2017/03/13, Maya Posch
*/


#include "co2_module.h"


// Static initialisations.
Timer CO2Module::timer;
uint8_t CO2Module::readCmd[9] = { 0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
uint8 CO2Module::eventLevel = 0; // 0: < 750, 1: > 900, > 1,000.
uint8 CO2Module::eventCountDown = 10; // Counter until lower event level.
uint8 CO2Module::eventCountUp = 0; // Counter until higher event level.


// --- INITIALIZE ---
bool CO2Module::initialize() {
	BaseModule::registerModule(MOD_IDX_CO2, CO2Module::start, CO2Module::shutdown);
}


// --- START ---
bool CO2Module::start() {
	// Register pins.
	if (!OtaCore::claimPin(ESP8266_gpio03)) { return false; } // RX 0
	if (!OtaCore::claimPin(ESP8266_gpio01)) { return false; } // TX 0
	
	// We use UART0 for the MH-Z14 CO2 sensor.
	// The sensor's UART uses 9600 baud, 8N1.
	//Serial.resetCallback();
	Serial.end();
	delay(10);
	Serial.begin(9600);
	Serial.setCallback(&CO2Module::onSerialReceived);
	
	// Create timer.
	// Read the current value every 30 seconds.
	// Since the preheat time of the sensor is 3 minutes, the first six
	// readings can theoretically be discarded. 
	timer.initializeMs(30000, CO2Module::readCO2).start();
	return true;
}


// --- SHUTDOWN ---
bool CO2Module::shutdown() {
	// Release pins pins.
	if (!OtaCore::releasePin(ESP8266_gpio03)) { return false; } // RX 0
	if (!OtaCore::releasePin(ESP8266_gpio01)) { return false; } // TX 0
	
	timer.stop();
	Serial.end();
	return true;
}


// --- READ CO2 ---
void CO2Module::readCO2() {
	// Write the command to the sensor which will make it send back the current
	// reading.
	Serial.write(readCmd, 9);
}


// --- CONFIG ---
// Processes any configuration commands for this module.
// Commands are provided in the following format:
// <command string>=<payload string>
void CO2Module::config(String cmd) {
	Vector<String> output;
	int numToken = splitString(cmd, '=', output);
	if (output[0] == "event" && numToken > 1) {
		// We got the configuration for events for this module. This can take
		// two forms: 
		// * output[1] is a '0', in which case events should be disabled.
		// * output[1] contains a series of values setting the trigger points.
		//
		// This module can be configured to publish events when the CO2 levels
		// reach certain levels:
		// * > T1 (e.g. 850 ppm for 10 samples), send a warning. 
		// * > T2 (e.g. 950 ppm for 10 samples), an error. 
		// * When dropping back below 750 ppm, send an 'OK'.
		
		// TODO: implement the configuration. Events are currently always on.
		//dhtPin = output[1].toInt();
	}
}


// --- ON SERIAL RECEIVED ---
// Callback for serial RX data events on UART 0.
void CO2Module::onSerialReceived(Stream &stream, char arrivedChar, unsigned short availableCharsCount) {
	//Serial1.printf("Receiving UART 0.\n");
	//OtaCore::log(LOG_DEBUG, "Receiving UART 0.");
	
	// Count until we have 9 characters available in the buffer.
	// Read them out, calculate the current PPM response and publish.
	if (availableCharsCount >= 9) {
		char buff[9];
		Serial.readBytes(buff, 9);
		
		int responseHigh = (int) buff[2];
		int responseLow = (int) buff[3];
		int ppm = (responseHigh * 0xFF) + responseLow;
		String response = OtaCore::getLocation() + ";" + ppm;
		OtaCore::publish("nsa/co2", response);
		
		// Check and update event counters.
		// Format of the semicolon-separated response is:
		// * location
		// * event level
		// * direction (0: level decrease, 1: increase)
		// * ppm level
		if (ppm > 1000) { // T3
			if (eventLevel < 2 && eventCountUp < 10) {
				if (++eventCountUp == 10) {
					// Send 'reached level 2' event.
					eventLevel = 2;
					eventCountDown = 0; // Reset.
					eventCountUp = 0;
					response = OtaCore::getLocation() + ";" + eventLevel + ";1;" + ppm;
					OtaCore::publish("nsa/events/co2", response);
				}
			}
		}
		else if (ppm > 850) { // T2
			if (eventLevel == 0 && eventCountUp < 10) {
				if (++eventCountUp == 10) {
					// Send 'reached level 1' event.
					eventLevel = 1;
					eventCountDown = 0;
					eventCountUp = 0;
					response = OtaCore::getLocation() + ";" + eventLevel + ";1;" + ppm;
					OtaCore::publish("nsa/events/co2", response);
				}
			}
			else if (eventLevel == 2 && eventCountDown < 10) {
				if (++eventCountDown == 10) {
					// Send 'reached level 1' event.
					eventLevel = 1;
					eventCountUp = 0;
					eventCountDown = 0;
					response = OtaCore::getLocation() + ";" + eventLevel + ";0;" + ppm;
					OtaCore::publish("nsa/events/co2", response);
				}
			}
		}
		else if (ppm < 750) { // T1
			if (eventLevel == 1 && eventCountDown < 10) {
				if (++eventCountDown == 10) {
					// Send 'reached level 0' event.
					eventLevel = 0;
					eventCountDown = 0;
					eventCountUp = 0;
					response = OtaCore::getLocation() + ";" + eventLevel + ";0;" + ppm;
					OtaCore::publish("nsa/events/co2", response);
				}
			}
		}
	}
}

/* void CO2Module::onSerialReceived(Stream &stream, char arrivedChar, unsigned short availableCharsCount) {
	//Serial1.printf("Receiving UART 0.\n");
	OtaCore::log(LOG_DEBUG, "Receiving UART 0.");
	
	// Count until we have 9 characters available in the buffer.
	// Read them out, calculate the current PPM response and publish.
	if (availableCharsCount >= 9) {
		char buff[9];
		Serial.readBytes(buff, 9);
		
		int responseHigh = (int) buff[2];
		int responseLow = (int) buff[3];
		int ppm = (responseHigh * 0xFF) + responseLow;
		String response = OtaCore::getMAC() + ";" + ppm;
		OtaCore::publish("nsa/co2", response);
	}
} */
