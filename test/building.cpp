/*
	building.cpp - Building class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#include "building.h"
#include "floor.h"



// --- CONSTRUCTOR ---
Building::Building(Config &config) {
	// The building configuration specifies how many floors we have. For each floor we create a 
	// new Floor instance, which we provide with a reference to the configuration we received.
	int floor_count = config.getValue<int>("Building.floors", 0);
	
	for (int i = 0; i < floor_count; ++i) {
		Floor floor(i + 1, config); // Floor numbering starts at 1.
		floors.push_back(floor);
	}
}


