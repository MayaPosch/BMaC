/*
	pwm_module.h - Header for the PWM Module class.
	
	Revision 0
	
	Features:
			- 
			
	2017/07/11, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef PWM_MODULE_H
#define PWM_MODULE_H


#include "ota_core.h"

#include <HardwarePWM.h>


class PwmModule {
	static HardwarePWM* hw_pwm;
	static Vector<int> duty;
	static uint8 pinNum;
	static Timer timer;
	//static Vector<int> pins;
	static uint8* pins;
	
public:
	static bool init();
	static bool shutdown();
	static void commandCallback(String message);
};


#endif
