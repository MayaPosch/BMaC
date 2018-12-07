/*
	config.h - Configuration file handler class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- Requires C++14 level features.
			- Uses the PocoProject INI parser, but alternatives can be used.
			
	2018/07/30, Maya Posch
*/


#include "config.h"


// --- CONSTRUCTOR ---
Config::Config() {
	parser = new IniFileConfiguration();
}


// --- LOAD ---
// Loads the provided INI configuration file.
// Return true on success, false on failure.
bool Config::load(std::string filename) {
	try {
		parser->load(filename);
	}
	catch (...) {
		// An exception has occurred. Return false.
		return false;
	}
	
	return true;
}

