/*
	base_module.h - Header for the Base Module class.
	
	Revision 0
	
	Features:
			- Declares the basic module management functions.
			
	2018/04/21, Maya Posch
*/


#include "base_module.h"


// Static initialisations.
BaseModule::SubModule BaseModule::modules[32];
uint32 BaseModule::active_mods = 0x0;
bool BaseModule::initialized = false;
uint8 BaseModule::modcount = 0;


// --- INIT ---
// Invokes the initialisation routine of any modules we wish to be using.
void BaseModule::init() {
	CO2Module::initialize();
	IOModule::initialize();
	JuraModule::initialize();
	JuraTermModule::initialize();
	MotionModule::initialize();
	PlantModule::initialize();
	PwmModule::initialize();
	SwitchModule::initialize();
	THPModule::initialize();
}


// --- REGISTER MODULE ---
bool BaseModule::registerModule(ModuleIndex index, modStart start, modShutdown shutdown) {
	// Initialise if necessary.
	if (!initialized) {
		for (uint8 i = 0; i < 32; i++) {
			modules[i].start = 0;
			modules[i].shutdown = 0;
			modules[i].index = index;
			modules[i].bitmask = (1 << i);
			modules[i].started = false;
		}

		initialized = true;
	}
	
	// Check whether we already have registered a module for this index.
	// If one has been registered, we reject this registration.
	if (modules[index].start) {
		return false;
	}
	
	// Add the module if it hasn't been registered yet.
	modules[index].start = start;
	modules[index].shutdown = shutdown;
	++modcount;
	
	return true;
}


// --- NEW CONFIG ---
bool BaseModule::newConfig(uint32 config) {
	OtaCore::log(LOG_DEBUG, String("Mod count: ") + String(modcount));
	
	// Compare the new configuration with the existing configuration.
	uint32 new_config = config ^ active_mods; // XOR comparison.
	if (new_config == 0x0) {
		OtaCore::log(LOG_INFO, "New configuration was 0x0. No change.");
		return true; 
	}
	
	OtaCore::log(LOG_INFO, "New configuration: " + new_config);
	
	// We got a bitmask with 32 positions, each corresponding to a position in the
	// modules array. An active position ('1') in the bitmask indicates that this module
	// has to be toggled from an 'on' to 'off' state or vice versa.
	//
	// For each '1' bit, check that a module has been registered in that position.
	// If it has, start or shut it down depending on its current state.
	for (uint8 i = 0; i < 32; ++i) {
		if (new_config & (1 << i)) {
			OtaCore::log(LOG_DEBUG, String("Toggling module: ") + String(i));
			if (modules[i].started) { 
				if ((modules[i]).shutdown()) { 
					modules[i].started = false; 
					active_mods ^= modules[i].bitmask;
				}
				else { 
					OtaCore::log(LOG_ERROR, "Failed to shutdown module.");
					return false; 
				}
			}
			else { 
				if ((modules[i].start) && (modules[i]).start()) { 
					modules[i].started = true;
					active_mods |= modules[i].bitmask;
				}
				else { 
					OtaCore::log(LOG_ERROR, "Failed to start module.");
					return false;
				}
			}
		}
	}
	
	return true;
}
