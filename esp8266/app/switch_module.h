/*
	switch_module.h - Header for the Switch Module class.
	
	Revision 0
	
	Features:
			- Allows one to control a latching relay-based switch.
			
	2017/11/29, Maya Posch
*/


#ifndef SWITCH_MODULE_H
#define SWITCH_MODULE_H


#include "base_module.h"



class SwitchModule {
	static String publishTopic;
	
public:
	static bool initialize();
	static bool start();
	static bool shutdown();
	static void commandCallback(String message);
};


#endif
