/*
	pwm_module.h - Header for the PWM Module class.
	
	Revision 0
	
	Features:
			- 
			
	2017/07/11, Maya Posch
*/


#ifndef PWM_MODULE_H
#define PWM_MODULE_H


#include "base_module.h"

#include <HardwarePWM.h>


class PwmModule {
	static HardwarePWM* hw_pwm;
	static Vector<int> duty;
	static uint8 pinNum;
	static Timer timer;
	//static Vector<int> pins;
	static uint8* pins;
	
public:
	static bool initialize();
	static bool start();
	static bool shutdown();
	static void commandCallback(String message);
};


#endif
