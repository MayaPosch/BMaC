/*
	jura_module.cpp - Jura Module class implementation.
	
	Revision 0
	
	Features:
			- Defines the Jura module class needed for serial comms functionality.
			
	2017/03/13, Maya Posch
*/


#include "jura_module.h"

#include <stdlib.h>


// Static initialisations.
Timer JuraModule::timer;
String JuraModule::mqttTxBuffer;

//String mqttTxBuffer;


// --- INITIALIZE ---
bool JuraModule::initialize() {
	BaseModule::registerModule(MOD_IDX_JURA, JuraModule::start, JuraModule::shutdown);
}


/* void onJuraSerialReceived(Stream &stream, char arrivedChar, unsigned short availableCharsCount) {
	//Serial1.printf("Receiving UART 0.\n");
	//OtaCore::log(LOG_DEBUG, "Receiving UART 0.");
	
	while(stream.available()){
		delay(1);
		uint8_t d0 = stream.read();
		delay(1);
		uint8_t d1 = stream.read();
		delay(1);
		uint8_t d2 = stream.read();
		delay(1);
		uint8_t d3 = stream.read();
		delay(7);
	  		
		// Decode and send the command to the first UART.
		//
		// Write payload bits into target byte.
		uint8_t d4;
		bitWrite(d4, 0, bitRead(d0, 2));
		bitWrite(d4, 1, bitRead(d0, 5));
		bitWrite(d4, 2, bitRead(d1, 2));
		bitWrite(d4, 3, bitRead(d1, 5));
		bitWrite(d4, 4, bitRead(d2, 2));
		bitWrite(d4, 5, bitRead(d2, 5));
		bitWrite(d4, 6, bitRead(d3, 2));
		bitWrite(d4, 7, bitRead(d3, 5));
		
		//Serial1.write(d4);
		//OtaCore::log(LOG_TRACE, String(d4));
		
		// Cache until end of response is detected (\n, LF), then send
		// via MQTT as well. May want to disable this somehow if the original
		// command was sent via UART 0 and not MQTT, and vice versa.
		mqttTxBuffer += (char) d4;
		if ('\n' == (char) d4) {
			// Read string in 16-bit sequences, each of which is a counter,
			// then publish the appropriate counters.
			long int espressoCount = strtol(mqttTxBuffer.substring(3, 7).c_str(), 0, 16);
			long int espresso2Count = strtol(mqttTxBuffer.substring(7, 11).c_str(), 0, 16);
			long int coffeeCount = strtol(mqttTxBuffer.substring(11, 15).c_str(), 0, 16);
			long int coffee2Count = strtol(mqttTxBuffer.substring(15, 19).c_str(), 0, 16);
			
			OtaCore::publish("nsa/espresso", OtaCore::getLocation() + ";" + espressoCount);
			OtaCore::publish("nsa/espresso2", OtaCore::getLocation() + ";" + espresso2Count);
			OtaCore::publish("nsa/coffee", OtaCore::getLocation() + ";" + coffeeCount);
			OtaCore::publish("nsa/coffee2", OtaCore::getLocation() + ";" + coffee2Count);
			mqttTxBuffer = "";
		}
	}
} */


// --- START ---
bool JuraModule::start() {
	// Register pins.
	if (!OtaCore::claimPin(ESP8266_gpio03)) { return false; }
	if (!OtaCore::claimPin(ESP8266_gpio01)) { return false; }
	
	// UART 0: Used for external UART device (coffee machine).
	// Runs at 9600 baud, 8N1.
	Serial.end();
	delay(10);
	Serial.begin(9600);
	Serial.setCallback(&JuraModule::onSerialReceived);
	
	// Callback for reading out statistics every minute, assuming that reading
	// more often is kind of ridiculous considering we're talking about a coffee
	// machine.
	timer.initializeMs(60000, JuraModule::readStatistics).start();
	
	return true;
}


// --- SHUTDOWN ---
bool JuraModule::shutdown() {
	// Release pins pins.
	if (!OtaCore::releasePin(ESP8266_gpio03)) { return false; } // RX 0
	if (!OtaCore::releasePin(ESP8266_gpio01)) { return false; } // TX 0
	
	timer.stop();
	Serial.end();
	
	return true;
}


// --- READ STATISTICS ---
void JuraModule::readStatistics() {
	// Send command to the coffee machine to read out the relevant memory.
	// 'RT:0000' => Read the first line of the EEPROM.
	String message = "RT:0000";
	JuraModule::toCoffeemaker(message);
}


// --- TO COFFEE MAKER ---
// Translate an ASCII character to 4 UART bytes and send them on UART 1.
bool JuraModule::toCoffeemaker(String cmd) {
	//Serial1.printf("Sending command %s.\n", cmd.c_str());
	OtaCore::log(LOG_DEBUG, "Sending command: " + cmd);
	
	// Post-fix CR LF.
	cmd += "\r\n";
	
	for (int i = 0; i < cmd.length(); ++i) {
		uint8_t ch = static_cast<uint8_t>(cmd[i]);
		uint8_t d0 = 0xFF;
		uint8_t d1 = 0xFF;
		uint8_t d2 = 0xFF;
		uint8_t d3 = 0xFF;
		
		// Reads each bit and writes it into the target bytes.
		// The Jura protocol puts a data bit on bits 2 & 5, with the rest of the
		// resulting four bytes all '1'.
		bitWrite(d0, 2, bitRead(ch, 0));
		bitWrite(d0, 5, bitRead(ch, 1));
		bitWrite(d1, 2, bitRead(ch, 2));
		bitWrite(d1, 5, bitRead(ch, 3));
		bitWrite(d2, 2, bitRead(ch, 4));
		bitWrite(d2, 5, bitRead(ch, 5));
		bitWrite(d3, 2, bitRead(ch, 6)); 
		bitWrite(d3, 5, bitRead(ch, 7));

		// Send the 4 bytes to the coffee maker.
		delay(1); 
		Serial.write(d0);
		delay(1); 
		Serial.write(d1);
		delay(1); 
		Serial.write(d2);
		delay(1); 
		Serial.write(d3);
		delay(7);
	}	

	return true;
}


// --- ON SERIAL RECEIVED ---
// Callback for serial RX data events on UART 0.
void JuraModule::onSerialReceived(Stream &stream, char arrivedChar, unsigned short availableCharsCount) {
	//Serial1.printf("Receiving UART 0.\n");
	OtaCore::log(LOG_DEBUG, "Receiving UART 0.");
	
	while(stream.available()){
		delay(1);
		uint8_t d0 = stream.read();
		delay(1);
		uint8_t d1 = stream.read();
		delay(1);
		uint8_t d2 = stream.read();
		delay(1);
		uint8_t d3 = stream.read();
		delay(7);
	  		
		// Decode and send the command to the first UART.
		//
		// Write payload bits into target byte.
		uint8_t d4;
		bitWrite(d4, 0, bitRead(d0, 2));
		bitWrite(d4, 1, bitRead(d0, 5));
		bitWrite(d4, 2, bitRead(d1, 2));
		bitWrite(d4, 3, bitRead(d1, 5));
		bitWrite(d4, 4, bitRead(d2, 2));
		bitWrite(d4, 5, bitRead(d2, 5));
		bitWrite(d4, 6, bitRead(d3, 2));
		bitWrite(d4, 7, bitRead(d3, 5));
		
		//Serial1.write(d4);
		OtaCore::log(LOG_TRACE, String(d4));
		
		// Cache until end of response is detected (\n, LF), then send
		// via MQTT as well. May want to disable this somehow if the original
		// command was sent via UART 0 and not MQTT, and vice versa.
		mqttTxBuffer += (char) d4;
		if ('\n' == (char) d4) {
			OtaCore::publish("coffee/response", OtaCore::getMAC() + ";" + mqttTxBuffer);
			mqttTxBuffer = "";
		}
	}
}
