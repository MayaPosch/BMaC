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
#include "sensor.h"
#include "actuator.h"

#include <vector>
#include <cstdint>


class Room {
	std::vector<Node> nodes;
	std::vector<Sensor> sensors;
	std::vector<Actuator> actuators;
	
public:
	Room(uint32_t type, Config &config);
	
};

#endif
