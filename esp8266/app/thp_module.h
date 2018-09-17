/*
	thp_module.h - Header for the Temperature/Humidity/Pressure module class.
	
	Revision 0
	
	Features:
			- Declares the basic class needed for this functionality.
			
	2017/05/10, Maya Posch
*/


#ifndef THP_MODULE_H
#define THP_MODULE_H


#include "base_module.h"


class THPModule {
	//
	
public:
	static bool initialize();
	static bool start();
	static bool shutdown();
	static void config(String cmd);
};


#endif
