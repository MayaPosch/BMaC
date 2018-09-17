/*
	switch_module.cpp - Implementation of the Switch Module class.
	
	Revision 0
	
	Features:
			- Allows one to control a latching relay-based switch.
			
	2017/11/29, Maya Posch
*/


#include "switch_module.h"

#include <Wire.h>


#define SW1_SET_PIN 5 	// GPIO 5 (D1 on NodeMCU).
#define SW2_SET_PIN 4 	// GPIO 4 (D2 on NodeMCU).
#define SW1_READ_PIN 14 // GPIO 14 (D5 on NodeMCU).
#define SW2_READ_PIN 12 // GPIO 12 (D6 on NodeMCU).


// Static initialisations.
String SwitchModule::publishTopic;


enum {
	SWITCH_ONE = 0x01,		// Switch the first connected load on, second off.
	SWITCH_TWO = 0x02,		// Switch the second connected load on, first off.
	SWITCH_STATE = 0x04,	// Returns position of the switch (0x01 or 0x02).
};


// --- INITIALIZE ---
bool SwitchModule::initialize() {
	BaseModule::registerModule(MOD_IDX_SWITCH, SwitchModule::start, SwitchModule::shutdown);
}


// --- START ---
// Create new class instance.
bool SwitchModule::start() {
	// Register pins.
	if (!OtaCore::claimPin(ESP8266_gpio05)) { return false; }
	if (!OtaCore::claimPin(ESP8266_gpio04)) { return false; }
	if (!OtaCore::claimPin(ESP8266_gpio14)) { return false; }
	if (!OtaCore::claimPin(ESP8266_gpio12)) { return false; }
	
	publishTopic = "switch/response/" + OtaCore::getLocation();
	OtaCore::registerTopic("switch/" + OtaCore::getLocation(), SwitchModule::commandCallback);
	
	// Set the pull-ups on the input pins and configure the output pins.
	pinMode(SW1_SET_PIN, OUTPUT);
	pinMode(SW2_SET_PIN, OUTPUT);
	pinMode(SW1_READ_PIN, INPUT_PULLUP);
	pinMode(SW2_READ_PIN, INPUT_PULLUP);
	
	digitalWrite(SW1_SET_PIN, LOW);
	digitalWrite(SW2_SET_PIN, LOW);
}


// --- SHUTDOWN ---
bool SwitchModule::shutdown() {
	OtaCore::deregisterTopic("switch/" + OtaCore::getLocation());
	
	// Release the pins.
	if (!OtaCore::releasePin(ESP8266_gpio05)) { return false; }
	if (!OtaCore::releasePin(ESP8266_gpio04)) { return false; }
	if (!OtaCore::releasePin(ESP8266_gpio14)) { return false; }
	if (!OtaCore::releasePin(ESP8266_gpio12)) { return false; }
}


// --- COMMAND CALLBACK ---
void SwitchModule::commandCallback(String message) {
	// Message is the command.
	OtaCore::log(LOG_DEBUG, "Switch command: " + message);
	
	// Format:
	// uint8	command
	//
	// Payload:
	// > 0x01 'switch one'
	// 
	// > 0x02 'switch two'
	//
	// > 0x04 'state'
	// 
	
	// Return value:
	// Each command echoes its own command code (e.g. 0x01) along with a boolean
	// value indicating success (1) or failure (0) in 2 uint8 bytes, except for the 
	// following commands. These return the command code (uint8) and a payload:
	// > 0x04
	// uint8	Result (0: failure, 1: success).
	// uint8	Active pin (0x00, 0x01).
	
	// Parse the message string.
	uint32 mlen = message.length();
	if (mlen < 1) { return; }
	int index = 0;
	uint8 cmd = *((uint8*) &message[index++]);
	if (cmd == SWITCH_ONE) {
		if (mlen > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Switching to position 1 failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x01 + (char) 0x00);
			return; 
		}
				
		// Set the relay to its first position (reset condition).
		// This causes pins 3 & 10 on the latching relay to become active.
		digitalWrite(SW1_SET_PIN, HIGH);
		delay(1000); // Wait 1 second for the relay to switch position.
		digitalWrite(SW1_SET_PIN, LOW);
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Switched to position 1.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x01 + (char) 0x01);
	}
	else if (cmd == SWITCH_TWO) {
		if (mlen > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Switching to position 2 failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x02 + (char) 0x00);
			return; 
		}
		
		// Set the relay to its first position (reset condition).
		// This causes pins 3 & 10 on the latching relay to become active.
		digitalWrite(SW2_SET_PIN, HIGH);
		delay(1000); // Wait 1 second for the relay to switch position.
		digitalWrite(SW2_SET_PIN, LOW);
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Switched to position 1.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x02 + (char) 0x01);
	}
	else if (cmd == SWITCH_STATE) {
		if (mlen > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading state failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x04 + (char) 0x00);
			return; 
		}
		
		// Check the value of the two input pins. If one is low, then that
		// is the active position.
		uint8 active = 2;
		if (digitalRead(SW1_READ_PIN) == LOW) { active = 0; }
		else if (digitalRead(SW2_READ_PIN) == LOW) { active = 1; }
		
		if (active > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading state failed: no active state found.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x04 + (char) 0x00);
			return; 
		}
		
		// Report success. QoS: 1.
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x04 + (char) 0x01 + 
												(char) active);
	}
}
