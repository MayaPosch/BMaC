/*
	floor.h - Floor class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#ifndef FLOOR_H
#define FLOOR_H


#include <vector>
#include <cstdint>

#include "room.h"


class Floor {
	std::vector<Room> rooms;
	
public:
	Floor(uint32_t level, Config &config);
};

#endif
