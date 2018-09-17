/*
	io_module.cpp - Implementation of the I/O Module class.
	
	Revision 0
	
	Features:
			- Allows one to control an i2c-enabled MCP23008 I/O expander.
			
	2017/11/15, Maya Posch
*/


#include "io_module.h"

#include <Wire.h>


// Static initialisations.
MCP23008* IOModule::mcp = 0;
uint8 IOModule::iodir;	// IO direction.
uint8 IOModule::gppu;	// Pull-up.
uint8 IOModule::gpio;	// GPIO state (high/low).
String IOModule::publishTopic;


enum {
	IO_START = 0x01,		// Activate this module.
	IO_STOP = 0x02,			// Deactivate this module.
	IO_STATE = 0x04,		// Returns I/O mode, pull-up & level state.
	IO_SET_MODE = 0x08,		// Set a pin to a specific mode (In/Out).
	IO_SET_PULLUP = 0x10,	// Set a pin's pull-up resistor (Low/High).
	IO_WRITE = 0x20,		// Set a pin to either Low or High.
	IO_READ = 0x40,			// Read the current pin value (Low, High).
	IO_ACTIVE = 0x80		// Return whether this module has been initialised.
};


enum {
	MCP_OUTPUT = 0,
	MCP_INPUT = 1
};


// --- INITIALIZE ---
bool IOModule::initialize() {
	BaseModule::registerModule(MOD_IDX_IO, IOModule::start, IOModule::shutdown);
}


// --- START ---
// Create new class instance.
bool IOModule::start() {	
	publishTopic = "io/response/" + OtaCore::getLocation();
	OtaCore::registerTopic("io/" + OtaCore::getLocation(), IOModule::commandCallback);
	
	// Start i2c comms.
	OtaCore::starti2c();
}


// --- SHUTDOWN ---
bool IOModule::shutdown() {
	OtaCore::deregisterTopic("io/" + OtaCore::getLocation());
	if (mcp) {
		delete mcp;
		mcp = 0;
	}
}


// --- COMMAND CALLBACK ---
void IOModule::commandCallback(String message) {
	// Message is the command.
	OtaCore::log(LOG_DEBUG, "I/O command: " + message);
	
	// Format:
	// uint8	command
	//
	// Payload:
	// > 0x01 'start'
	// uint8	MPC23008 i2c address (optional: defaults to 0x20): 0 - 7.
	// 
	// > 0x02 'stop'
	//
	// > 0x04 'state'
	// 
	// > 0x08 ' set mode'
	// uint8	Pin number (0 - 7 for MCP23008).
	// uint8	Pin state: 0 - Output, 1 - Input.
	//
	// > 0x10 'set pull-up'
	// uint8	Pin number (0 - 7).
	// uint8	Pin pull-up state: 0 - Low, 1 - High.
	//
	// > 0x20 'write'
	// uint8	Pin number (0 - 7).
	// uint8	Value to set the pin to: 0 - Low, 1 - High.
	//
	// > 0x40 'read'
	// uint8	Pin number (0 - 7).
	//
	// > 0x80 'active'
	//
	
	// Return value:
	// Each command echoes its own command code (e.g. 0x01) along with a boolean
	// value indicating success (1) or failure (0) in 2 uint8 bytes, except for the 
	// following commands. These return the command code (uint8) and a payload:
	// > 0x04
	// uint8	Result (0: failure, 1: success).
	// uint8	Pin mode (0: Output, 1: Input). One bit per pin.
	// uint8	Pull-up state (0: Low, 1: High). One bit per pin.
	// uint8	Level state (0: Low, 1: High). One bit per pin.
	// 
	// > 0x40
	// uint8	Result (0: failure, 1: success).
	// uint8	Pin # (0 - 7).
	// uint8	Pin value (0: Low, 1: High).
	//
	// > 0x80
	// uint8	Result.
	// uint8	Current state (0: not initialised, 1: initialised).
	
	// Parse the message string.
	uint32 mlen = message.length();
	if (mlen < 1) { return; }
	int index = 0;
	uint8 cmd = *((uint8*) &message[index++]);
	if (cmd == IO_START) {
		if (mlen > 2) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Enabling I/O Module failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x01 + (char) 0x00);
			return; 
		}
		
		// Read out the desired address, or use the default.
		uint8 addr = 0;
		if (mlen == 2) {
			addr = *((uint8*) &message[index++]);
			if (addr > 7) {				
				// Report failure. QoS 1.
				OtaCore::log(LOG_INFO, "Enabling I/O Module failed: invalid i2c address.");
				OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x01 + (char) 0x00);
				return;
			}
		}
			
		if (!mcp) {
			// FIXME: lib requires 8-bit address. Make this work with custom
			// addresses (other than 0x20).
			mcp = new MCP23008(0x40);
		}		
	
		// Set all pins to output (0) and low (0)
		mcp->writeIODIR(0x00);
		mcp->writeGPIO(0x00);
		
		// Read in current chip values.
		iodir = mcp->readIODIR();
		gppu = mcp->readGPPU();
		gpio = mcp->readGPIO();
		
		// Validate IODIR and GPIO registers.
		if (iodir != 0 || gpio != 0) {
			delete mcp;
			mcp = 0;
			
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Enabling I/O Module failed: not connected.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x01 + (char) 0x00);
			return;
		}
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Enabled I/O Module.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x01 + (char) 0x01);
	}
	else if (cmd == IO_STOP) {
		if (mlen > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Disabling I/O Module failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x02 + (char) 0x00);
			return; 
		}
		
		if (mcp) {
			delete mcp;
			mcp = 0;
		}
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Disabled I/O Module.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x02 + (char) 0x01);
	}
	else if (cmd == IO_STATE) {
		if (mlen > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading state failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x04 + (char) 0x00);
			return; 
		}
		
		// Report success. QoS: 1.
		//char output[] = { 0x04, 0x01, (char) iodir, (char) gppu, (char) gpio };
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x04 + (char) 0x01 + 
												//String((const char*) output, 5));
												((char) iodir) + ((char) gppu) +
												((char) gpio));
	}
	else if (cmd == IO_SET_MODE) {
		if (mlen != 3) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading state failed: incorrect number of parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x08 + (char) 0x00);
			return; 
		}
		
		uint8 pnum = *((uint8*) &message[index++]);
		uint8 pstate = *((uint8*) &message[index]);
		if (pnum > 7) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Setting pin mode failed: unknown pin.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x08 + (char) 0x00);
			return; 
		}
		
		if (pstate > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Setting pin mode failed: invalid pin mode.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x08 + (char) 0x00);
			return; 
		}
		
		// Set new state of IODIR register.
		if (pstate == MCP_INPUT) { iodir |= 1 << pnum; } 
		else { iodir &= ~(1 << pnum); }
		
		if (mcp) {
			OtaCore::log(LOG_DEBUG, "Setting pinmode in library...");
			mcp->writeIODIR(iodir);
		}
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Set pin mode for I/O Module.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x08 + (char) 0x01);
	}
	else if (cmd == IO_SET_PULLUP) {		
		if (mlen != 3) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading state failed: incorrect number of parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x10 + (char) 0x00);
			return; 
		}
		
		// Set the new pull-up status.
		uint8 pnum = *((uint8*) &message[index++]);
		uint8 pstate = *((uint8*) &message[index]);
		if (pnum > 7) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Setting pull-up failed: unknown pin.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x10 + (char) 0x00);
			return; 
		}
		
		if (pstate > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Setting pull-up failed: invalid state.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x10 + (char) 0x00);
			return; 
		}
		
		// Set new state of the GPPU register.
		if (pstate == HIGH) { gppu |= 1 << pnum; } 
		else { gppu &= ~(1 << pnum); }
		
		if (mcp) {
			OtaCore::log(LOG_DEBUG, "Setting pull-up in library...");
			mcp->writeGPPU(gppu);
		}
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Changed pull-up for I/O Module.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x10 + (char) 0x01);
	}
	else if (cmd == IO_WRITE) {
		if (mlen != 3) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Writing pin failed: incorrect number of parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x20 + (char) 0x00);
			return; 
		}
		
		// Set the new GPIO pin level.
		uint8 pnum = *((uint8*) &message[index++]);
		uint8 pstate = *((uint8*) &message[index]);
		if (pnum > 7) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Writing pin failed: unknown pin.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x20 + (char) 0x00);
			return; 
		}
		
		if (pstate > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Writing pin failed: invalid state.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x20 + (char) 0x00);
			return; 
		}
		
		// Set the new state of the GPIO register.
		String state = "low";
		if (pstate == HIGH) { gpio |= 1 << pnum; state = "high"; } 
		else { gpio &= ~(1 << pnum); }
		
		OtaCore::log(LOG_DEBUG, "Changed GPIO to: " + ((char) gpio));
		
		if (mcp) {
			OtaCore::log(LOG_DEBUG, "Setting state to " + state + 
							" in library for pin " + ((char) pnum));
			mcp->writeGPIO(gpio);
		}
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Wrote pin state for I/O Module.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x20 + (char) 0x01);
	}
	else if (cmd == IO_READ) {
		if (mlen > 2) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading pin failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x40 + (char) 0x00);
			return; 
		}
		
		// Read the GPIO pin status and return it.
		uint8 pnum = *((uint8*) &message[index]);
		if (pnum > 7) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading pin failed: unknown pin.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x40 + (char) 0x00);
		}
		
		uint8 pstate;
		if (mcp) {
			OtaCore::log(LOG_DEBUG, "Reading pin in library...");
			pstate = (mcp->readGPIO() >> pnum) & 0x1;
		}
		
		// Report success. QoS: 1.
		OtaCore::log(LOG_INFO, "Read pin state for I/O Module.");
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x40 + (char) 0x01 + 
												(char) pnum + (char) pstate);
	}
	else if (cmd == IO_ACTIVE) {
		if (mlen > 1) {
			// Report failure. QoS 1.
			OtaCore::log(LOG_INFO, "Reading active status failed: too many parameters.");
			OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												(char) 0x80 + (char) 0x00);
			return; 
		}
		
		uint8 active = 0;
		if (mcp) { active = 1; }
		
		// Report success. QoS: 1.
		char output[] = { 0x80, 0x01, active };
		OtaCore::publish(publishTopic, OtaCore::getLocation() + ";" + 
												//(char) 0x80 + (char) 0x01 + 
												String(output, 3));
												//(char) active);
	}
}
