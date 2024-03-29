/*
	plant_module.cpp - Plant Module class implementation.
	
	Revision 0
	
	Features:
			- Measures soil moisture level via the sensor on the ADC pin.
			- Publishes read values via OtaCore::publish().
			
	2018/04/21, Maya Posch
*/


#include "plant_module.h"


// Static initialisations.
int PlantModule::pin = PLANT_GPIO_PIN;
Timer PlantModule::timer;
uint16 PlantModule::humidityTrigger = 530;
String PlantModule::publishTopic;
HttpServer PlantModule::server;
APA102* PlantModule::LED = 0;
HX711* PlantModule::scale = 0;


enum {
	PLANT_SOIL_MOISTURE = 0x01,
	PLANT_SET_TRIGGER = 0x02,
	PLANT_TRIGGER = 0x04
};


// --- INITIALIZE ---
bool PlantModule::initialize() {
	BaseModule::registerModule(MOD_IDX_PLANT, PlantModule::start, PlantModule::shutdown);
	
	return true;
}


// --- START ---
bool PlantModule::start() {
	OtaCore::log(LOG_INFO, "Plant Module starting...");
	
	// Register pins.
	if (!OtaCore::claimPin(pin)) { return false; }
	
	publishTopic = MQTT_PREFIX + String("plant/response/") + OtaCore::getLocation();
	OtaCore::registerTopic(MQTT_PREFIX + String("plants/") + OtaCore::getLocation(), PlantModule::commandCallback);
	
	// Set output pin mode.
	pinMode(pin, OUTPUT);
	
	// Start the webserver.
	server.listen(80);
	//server.addPath("/", onIndex);
	server.paths.setDefault(PlantModule::onRequest);
	
	// Set the APA102 RGB LED to display green.
	LED = new APA102(NUM_APA102);
	LED->setBrightness(15); // 0 - 31
	LED->clear();
	LED->setAllPixel(0, 255, 0); // RGB
	LED->show();
	
	// Set up the HX711-based scale instance.
	// Use pins: 
	// GPIO 1: Data.
	// GPIO 3: Clock.
	// gain: 128 or 64 for channel A; channel B works with 32 gain factor only
	if (!OtaCore::claimPin(ESP8266_gpio01)) { return false; }
	if (!OtaCore::claimPin(ESP8266_gpio03)) { return false; }
	scale = new HX711();
	scale->begin(1, 3, 128);
	
	// Create timer.
	// Read the current value every 60 seconds.
	timer.initializeMs(60000, PlantModule::readSensor).start();
	return true;
}


// --- SHUTDOWN ---
bool PlantModule::shutdown() {
	// Release pins.
	if (!OtaCore::releasePin(pin)) { return false; }
	
	// Stop webserver.
	server.shutdown();
	
	// Delete APA102 object.
	if (LED) {
		delete LED;
		LED = 0;
	}
	
	if (scale) {
		delete scale;
		scale = 0;
	}
	
	OtaCore::deregisterTopic(MQTT_PREFIX + String("plants/") + OtaCore::getLocation());
	
	timer.stop();
	return true;
}


// --- COMMAND CALLBACK ---
void PlantModule::commandCallback(String message) {
	// Message is the command.
	OtaCore::log(LOG_DEBUG, "Plant command: " + message);
	
	// Format:
	// uint8	command
	//
	// Payload:
	// > 0x01 'soil moisture'
	//
	// > 0x02 'set trigger'
	// uint16	New trigger point.
	//
	// > 0x04 'trigger'
	// 
	
	// Return value:
	// Each command echoes its own command code (e.g. 0x01) along with a boolean
	// value indicating success (1) or failure (0) in 2 uint8 bytes, except for the 
	// following commands. These return the command code (uint8) and a payload:
	// > 0x01
	// uint8	Result (0: failure, 1: success).
	// uint16	Current sensor value.
	//
	// > 0x04
	// uint8	Result (0: failure, 1: success).
	// uint16	Current trigger point.
	
	// Parse the message string.
	if (message.length() < 1) { return; }
	int index = 0;
	uint8 cmd = *((uint8*) &message[index++]);
	
	if (cmd == PLANT_SOIL_MOISTURE) {
		// Perform sensor read.
		readSensor();
	}
	else if (cmd == PLANT_SET_TRIGGER) {
		// Get new value from the payload. Perform sanity check.
		if (message.length() != 3) { return; }
		uint16 payload = *((uint16*) &message[index]);
		index += 2;
		
		// TODO: add sanity.
		humidityTrigger = payload;
	}
	else if (cmd == PLANT_TRIGGER) {
		// Return current trigger point.
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" 
										+ String(((char*) &humidityTrigger), 2));
	}
}


// --- READ SENSOR ---
void PlantModule::readSensor() {
	// Read out the scale value.
	if (scale->is_ready()) {
		long weight = scale->read();
		
		// Publish reading.
		OtaCore::publish(MQTT_PREFIX"nsa/plant/scale_weight_raw", OtaCore::getLocation() + ";" + 
																String(weight));
	}
	
	// Read the analogue (ADC) pin for its current value and publish this value.
	int16_t val = 0;
	val = analogRead(A0); // calls system_adc_read().
	
	String response = OtaCore::getLocation() + ";" + val;
	OtaCore::publish(MQTT_PREFIX"nsa/plant/response", response);
	OtaCore::publish(MQTT_PREFIX"nsa/plant/moisture_raw", response);
	
	// Calculate the humidity %.
	//int16_t percentage = 100 - ((val - 379) / 3.87);
	
	// Check whether we need to water the plant.
	// Here the set plant profile has the following variables:
	// * Dry period days (count).
	// * Minimum moisture.
	// * Maximum moisture.
	
	// Calibrated values are 766 for air (0%), 379 for water (100%). 1% is therefore ~3.87.
	// Percentage = 100 - ((measured - water) / 3.87)
	// Target range is: 40-60% (154 - 232)
	// TODO: make dynamically configurable.
	// 50% is 193 + water => 572
	// 60% is 154 + water => 533
	if (val >= humidityTrigger) {
		// We're at 50% or lower.
		// Activate the pump for a couple of seconds.
		// The pump is connected to GPIO 5.
		// TODO: make pump GPIO configurable.
		digitalWrite(pin, HIGH);
		
		// Pulsate the APA102 LED blue while the pump is active.
		// First set colour to blue.
		LED->setBrightness(31);
		LED->setAllPixel(0, 0, 255);
		LED->show();
		
		// Fade and increase brightness over ten seconds.
		for (int i = 0; i < 10; ++i) {
			LED->directWrite(0, 0, 255, 25);
			delay(200);
			LED->directWrite(0, 0, 255, 18);
			delay(200);
			LED->directWrite(0, 0, 255, 12);
			delay(200);
			LED->directWrite(0, 0, 255, 5);
			delay(200);
			LED->directWrite(0, 0, 255, 31);
			delay(200);
		}
		
		digitalWrite(pin, LOW);
	}
}


void PlantModule::onRequest(HttpRequest& request, HttpResponse& response) {
	// TODO: update to current API.
	/* TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	TemplateVariables& vars = tmpl->variables();
	int16_t val = analogRead(A0);
	int8_t perc = 100 - ((val - 379) / 3.87);
	vars["raw_value"] = String(val);
	vars["percentage"] = String(perc);
	
	response.sendNamedStream(tmpl); // will be automatically deleted */
}
