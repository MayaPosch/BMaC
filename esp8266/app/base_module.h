/*
	base_module.h - Header for the Base Module class.
	
	Revision 0
	
	Features:
			- Declares the basic module management functions.
			
	2018/04/21, Maya Posch
*/


#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include "ota_core.h"


// Enums
enum ModuleType {
	MOD_TEMPERATURE_HUMIDITY = 0x1,
	MOD_CO2 = 0x2,
	MOD_JURA = 0x4,
	MOD_JURATERM = 0x8,
	MOD_MOTION = 0x10,
	MOD_PWM = 0x20,
	MOD_IO = 0x40,
	MOD_SWITCH = 0x80,
	MOD_PLANT = 0x100
};

enum ModuleIndex {
	MOD_IDX_TEMPERATURE_HUMIDITY = 0,
	MOD_IDX_CO2,
	MOD_IDX_JURA,
	MOD_IDX_JURATERM,
	MOD_IDX_MOTION,
	MOD_IDX_PWM,
	MOD_IDX_IO,
	MOD_IDX_SWITCH,
	MOD_IDX_PLANT
};


typedef bool (*modStart)();
typedef bool (*modShutdown)();


// Include the module headers.
#include "thp_module.h"
#include "jura_module.h"
#include "juraterm_module.h"
#include "co2_module.h"
#include "motion_module.h"
#include "pwm_module.h"
#include "io_module.h"
#include "switch_module.h"
#include "plant_module.h" 


class BaseModule {	
	struct SubModule {
		modStart start;
		modShutdown shutdown;
		ModuleIndex index;
		uint32 bitmask;
		bool started;
	};
	
	static SubModule modules[32];
	static uint32 active_mods;
	static bool initialized;
	static uint8 modcount;
	
public:
	static void init();
	static bool registerModule(ModuleIndex index, modStart start, modShutdown shutdown);
	static bool newConfig(uint32 config);
	static uint32 activeMods() { return active_mods; }
};


#endif
