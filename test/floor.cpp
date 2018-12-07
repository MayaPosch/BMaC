/*
	floor.cpp - Floor class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/

#include "floor.h"
#include "utility.h"

#include <string>


// --- CONSTRUCTOR ---
Floor::Floor(uint32_t level, Config &config) {
	// For each floor, create the room instances defined for it. 
	std::string floor_cat = "Floor_" + std::to_string(level);
	std::string roomsStr = config.getValue<std::string>(floor_cat + ".rooms", 0);
	
	// Extract the room IDs.
	std::vector<std::string> room_ids;
	split_string(roomsStr, ',', room_ids);	
	int room_count = room_ids.size();
	
	// Create the rooms for this floor.
	if (room_count > 0) {	
		for (int i = 0; i < room_count; ++i) {
			Room room(std::stoi(room_ids.at(i)), config);
			rooms.push_back(room);
		}
	}
}
