/*
	motion_module.cpp - MotionModule class implementation.
	
	Revision 0
	
	Features:
			- Defines the motion module class needed for ESP8266 functionality.
			- Implements PIR sensor monitoring functionality.
			
	2017/05/10, Maya Posch
*/


#include "motion_module.h"


// Static initialisations.
int MotionModule::pin = GPIO_PIN;
Timer MotionModule::timer;
Timer MotionModule::warmup;
bool MotionModule::motion = false;
bool MotionModule::firstLow = true;


// --- INITIALIZE ---
bool MotionModule::initialize() {
	BaseModule::registerModule(MOD_IDX_MOTION, MotionModule::start, MotionModule::shutdown);
}


// --- START ---
bool MotionModule::start() {
	// Register pins.
	if (!OtaCore::claimPin(ESP8266_gpio00)) { return false; }
	
	// The PIR sensor is connected to GPIO 0 (D3 on NodeMCU) by default.
	// First set the sensor pin to INPUT mode.
	pinMode(pin, INPUT);
	
	// Perform sensor warmup for 1 minute.
	warmup.initializeMs(60000, MotionModule::warmupSensor).start();
	
	return true;
}


// --- SHUTDOWN ---
bool MotionModule::shutdown() {
	// Release pins pins.
	if (!OtaCore::releasePin(ESP8266_gpio00)) { return false; } // RX 0
	
	// Detach the interrupt and stop the timer.
	timer.stop();
	detachInterrupt(pin);
	
	return true;
}


// --- CONFIG ---
// Processes any configuration commands for this module.
// Commands are provided in the following format:
// <command string>=<payload string>
void MotionModule::config(String cmd) {
	Vector<String> output;
	int numToken = splitString(cmd, '=', output);
	if (output[0] == "set_pin" && numToken > 1) {
		//pin = output[1].toInt();
		
		// TODO: 
	}
}


// --- WARMUP SENSOR ---
// Called after the sensor has warmed up and is ready for readings.
void MotionModule::warmupSensor() {
	warmup.stop();
	
	// Set an interrupt on the motion sensor pin to listen for changes.
	attachInterrupt(pin, &MotionModule::interruptHandler, CHANGE);
	
	// We want to publish the current state of the pin every 5 seconds, for 
	// which we need to set the timer. The actual reading of the pin is done
	// by the interrupt routine. This timer thus reports the current value
	// set by that interrupt routine.
	timer.initializeMs(5000, MotionModule::readSensor).start();
}


// --- READ SENSOR ---
//
void MotionModule::readSensor() {
	// Since the goal is to determine absence of people in a room, we wish to 
	// ignore brief periods of the sensor reporting LOW. To do this, we use a 
	// single timer which polls every 5 seconds. If LOW is read the first time,
	// it is ignored. The second time LOW is reported.
	//
	// The sensor in question (HC-SR501) has a recovery time (after triggering)
	// of at least 5 seconds. This means that during brief quiet periods in a
	// room it can go LOW. By validating this LOW value using a second LOW value
	// measured in the same 10-second period, we can be fairly certain that it's
	// the actual value.
	
	// Detect changes to report.
	if (!motion) {
		// Check whether this is the first low report.
		if (firstLow) { firstLow = false; }
		else {		
			// Report motion stopped.
			OtaCore::publish("nsa/motion", OtaCore::getLocation() + ";0");
			firstLow = true;
		}
	}
	else if (motion) {
		// Report motion.
		OtaCore::publish("nsa/motion", OtaCore::getLocation() + ";1");
		firstLow = true;
	}
}


// --- INTERRUPT HANDLER ---
// Called when the value of the monitored GPIO pin changes.
void IRAM_ATTR MotionModule::interruptHandler() {
	// Set boolean value depending on what has been read.
	int val = digitalRead(pin);
	if (val == HIGH) { motion = true; }
	else { motion = false; }
}
