/*
	room.h - Room class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#ifndef ROOM_H
#define ROOM_H


#include "node.h"
#include "device.h"

#include <vector>
#include <map>
#include <cstdint>


class Room {
	std::map<std::string, Node> nodes;
	std::vector<Device> sensors;
	std::vector<Device> actuators;
	
public:
	Room(uint32_t type, Config &config);
	
};

#endif
