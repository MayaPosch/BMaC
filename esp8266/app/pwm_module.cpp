/*
	pwm_module.cpp - PWM Module class implementation.
	
	Revision 0
	
	Features:
			- Allows for the control of PWM pins.
			
	2017/07/11, Maya Posch
*/


#include "pwm_module.h"


// Static initialisations.
HardwarePWM* PwmModule::hw_pwm = 0;
uint8 PwmModule::pinNum = 0;
Timer PwmModule::timer;
uint8* PwmModule::pins = 0;


enum {
	PWM_START = 0x01,
	PWM_STOP = 0x02,
	PWM_SET_DUTY = 0x04,
	PWM_DUTY = 0x08,
	PWM_ACTIVE = 0x10
};


// --- INITIALIZE ---
bool PwmModule::initialize() {
	BaseModule::registerModule(MOD_IDX_PWM, PwmModule::start, PwmModule::shutdown);
}


// --- START ---
bool PwmModule::start() {
	OtaCore::registerTopic(MQTT_PREFIX + String("pwm/") + OtaCore::getLocation(), PwmModule::commandCallback);
	
	return true;
}


// --- SHUTDOWN ---
bool PwmModule::shutdown() {
	OtaCore::deregisterTopic(MQTT_PREFIX + String("pwm/") + OtaCore::getLocation());
	
	// Clean up resources.
	if (hw_pwm) {
		delete hw_pwm;
		hw_pwm = 0;
	}
	
	return true;
}


// --- COMMAND CALLBACK ---
void PwmModule::commandCallback(String message) {
	// Message is the command.
	OtaCore::log(LOG_DEBUG, "PWM command: " + message);
	
	// Format:
	// uint8	command
	// 
	// Payload:
	// > 0x01 'start'
	// uint8	number of pin numbers
	// uint8(*)	GPIO pin numbers
	// 
	// > 0x04 'set duty'
	// uint8	GPIO pin number
	// uint8	duty % (0-100)
	//
	// > 0x08 'duty'
	// uint8	GPIO pin number
	
	// Parse the message string.
	if (message.length() < 1) { return; }
	int index = 0;
	uint8 cmd = *((uint8*) &message[index++]);
		
	if (cmd == PWM_START) {
		// next uint8 in message contains number of pins.
		// The message string then has to be long enough for those pins.
		if (message.length() < 2) { return; }
		uint8 num = *((uint8*) &message[index++]);
		
		OtaCore::log(LOG_DEBUG, "Pins to add: " + String(num));
		
		if (message.length() != (2 + num)) { return; }
		
		pins = new uint8[num];
		for (int i = 0; i < num; ++i) {
			pins[i] = *((uint8*) &message[index++]);
			if (!OtaCore::claimPin(pins[i])) {
				OtaCore::log(LOG_ERROR, "Pin is already in use: " + String(pins[i]));
				
				// Report failure. QoS: 1.
				OtaCore::publish("pwm/response", OtaCore::getLocation() + ";0", 1);
				
				return; 
			}
			
			OtaCore::log(LOG_INFO, "Adding GPIO pin " + String(pins[i]));
		}
		
		hw_pwm = new HardwarePWM(pins, num);
		pinNum = num;
		
		OtaCore::log(LOG_INFO, "Added pins to PWM: " + String(pinNum));
		
		// Report success. QoS: 1.
		OtaCore::publish("pwm/response", OtaCore::getLocation() + ";1", 1);
		
		// Start heartbeat timer to provide active feedback?
	}
	else if (cmd == PWM_STOP) {
		// FIXME: duty is not reset when simply destroying the HW PWM object. Maybe reset to 0?
		delete hw_pwm;
		hw_pwm = 0;
		
		// Release GPIO pins.
		for (int i = 0; i < pinNum; ++i) {
			if (!OtaCore::releasePin(pins[i])) {
				OtaCore::log(LOG_ERROR, "Pin cannot be released: " + String(pins[i]));
				
				// Report failure. QoS: 1.
				OtaCore::publish("pwm/response", OtaCore::getLocation() + ";0", 1);
				
				return; 
			}
			
			OtaCore::log(LOG_INFO, "Adding GPIO pin " + String(pins[i]));
		}
		
		delete[] pins;
		pins = 0;
		
		// Report success.
		OtaCore::publish("pwm/response", OtaCore::getLocation() + ";1");
	}
	else if (cmd == PWM_SET_DUTY) {
		if (message.length() < 3) { return; }
		
		// FIXME: check for valid (active) pin. (or !boom!)
		uint8 pin = *((uint8*) &message[index++]);
		uint8 duty = *((uint8*) &message[index++]);
		bool ret = hw_pwm->setDuty(pin, ((uint32) 222.22 * duty));
		if (!ret) {
			// Report error.
			OtaCore::publish("pwm/response", OtaCore::getLocation() + ";0");
			
			return;
		}
		
		// Report success.
		OtaCore::publish("pwm/response", OtaCore::getLocation() + ";1");
	}
	else if (cmd == PWM_DUTY) {
		if (message.length() < 2) { return; }
		
		uint8 pin = *((uint8*) &message[index++]);
		uint32 duty = hw_pwm->getDuty(pin);
		
		// Convert back to % and publish.
		uint8 dutyp = (duty / 222.22) + 1;
		String res = "";
		res += (char) pin;
		res += (char) dutyp;
		OtaCore::publish("pwm/response", OtaCore::getLocation() + ";" + res);
	}
	else if (cmd == PWM_ACTIVE) {
		// Return list of active pins.
		String res;
		if (pins && pinNum > 0) {
			res = String((char*) pins, pinNum); // Pins array to string.
		}
		
		OtaCore::publish("pwm/response", OtaCore::getLocation() + ";" + res);
	}
}
