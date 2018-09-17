/*
	io_module.h - Header for the I/O Module class.
	
	Revision 0
	
	Features:
			- Allows one to control an i2c-enabled MCP23008 I/O expander.
			
	2017/11/15, Maya Posch
*/


#ifndef IO_MODULE_H
#define IO_MODULE_H


#include "base_module.h"

#include <Libraries/MCP23008/MCP23008.h>


class IOModule {
	static MCP23008* mcp;
	static uint8 iodir;	// IO direction.
	static uint8 gppu;	// Pull-up.
	static uint8 gpio;	// GPIO state (high/low).
	static String publishTopic;
	
public:
	static bool initialize();
	static bool start();
	static bool shutdown();
	static void commandCallback(String message);
};


#endif
