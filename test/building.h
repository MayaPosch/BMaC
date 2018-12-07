/*
	building.h - Building class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#ifndef BUILDING_H
#define BUILDING_H


#include <vector>
#include <string>

#include "floor.h"


class Building {
	std::vector<Floor> floors;
	
public:
	Building(Config &cfg);
};

#endif
