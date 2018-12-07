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
#include "devices/device.h"

#include <vector>
#include <map>
#include <cstdint>


class Room {
	std::map<std::string, Node> nodes;
	std::vector<Device> devices;
	std::shared_ptr<RoomState> state;
	
public:
	Room(uint32_t type, Config &config);
	
};

#endif
