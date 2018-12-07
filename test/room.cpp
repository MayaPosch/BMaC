/*
	room.cpp - Room class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#include "room.h"
//#include "device_types.h"

#include "utility.h"


// --- CONSTRUCTOR ---
Room::Room(uint32_t type, Config &config) {
	// Get the configuration for this room instance from the configuration using the type number.
	// Create instances of the specified devices.
	std::string room_cat = "Room_" + std::to_string(type);
	std::string nodeStr = config.getValue<std::string>(room_cat + ".nodes", "");
	
	// Set initial room status values.
	state->setTemperature(24.3);
	state->setHumidity(51.2);
	
	// A node configuration consists out of a Node instance, the peripherals it attaches to
	// and the interfaces between both.
	
	// First create a Node instance, then the peripherals, finally the appropriate interface
	// instance, adding the interface to both to create the link.
	std::string sensors;
	std::string actuators;
	std::string node_cat;
	if (!nodeStr.empty()) {
		// Extract the node IDs.
		std::vector<std::string> node_ids;
		split_string(nodeStr, ',', node_ids);
		int node_count = node_ids.size();
		
		// Create the nodes.
		for (int i = 0; i < node_count; ++i) {
			Node node(node_ids.at(i), config);	
			node_cat = "Node_" + node_ids.at(i);			
			nodes.insert(std::map<std::string, Node>::value_type(node_ids.at(i), node));
		}
		
		std::string devicesStr = config.getValue<std::string>(node_cat + ".devices", "");
		if (!devicesStr.empty()) {
			// Extract the device IDs.
			std::vector<std::string> device_ids;
			split_string(devicesStr, ':', device_ids);
			int device_count = device_ids.size();
			
			// Create the devices.
			for (int i = 0; i < device_count; ++i) {
				// Split the device section into its ID and the node it is to be connected to.
				std::vector<std::string> device_data;
				split_string(device_ids.at(i), ':', device_data);
				if (device_data.size() != 2) {
					// Incorrect data. Abort.
					continue;
				}
				
				// Create the new sensor.
				Device device(device_data[0], config, state);
				
				// Add sensor to the node.
				nodes.at(device_data[1]).addDevice(std::move(device));
				
				devices.push_back(device);
			}
		}
	}
	
}
