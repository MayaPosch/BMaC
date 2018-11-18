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
	std::string nodes = config.getValue<std::string>(room_cat + ".nodes", "");
	
	// A node configuration consists out of a Node instance, the peripherals it attaches to
	// and the interfaces between both.
	
	// First create a Node instance, then the peripherals, finally the appropriate interface
	// instance, adding the interface to both to create the link.
	std::string sensors;
	std::string actuators;
	std::string node_cat;
	if (!nodes.empty()) {
		// Extract the node IDs.
		std::vector<std::string> node_ids;
		split_string(nodes, ',', node_ids);	
		int node_count = node_ids.size();
		
		// Create the nodes.
		for (int i = 0; i < node_count; ++i) {
			Node node(std::stoi(node_ids.at(i)), config);
			
			node_cat = "Node_" + std::to_string(i + 1);
			
			nodes.push_back(node);
		}
		
		sensors = config.getValue<std::string>(node_cat + ".sensors", "");
		actuators = config.getValue<std::string>(node_cat + ".actuators", "");	
		if (!sensors.empty()) {
			// Extract the sensor IDs.
			std::vector<std::string> sensor_ids;
			split_string(sensors, ',', sensor_ids);
			int sensor_count = sensor_ids.size();
			
			// Create the sensors.
			for (int i = 0; i < sensor_count; ++i) {
				// Split the sensor section into its ID and the node it is to be connected to.
				std::vector<std::string> sensor_data;
				split_string(sensor_ids.at(i), sensor_data);
				if (sensor_data.size() != 2) {
					// Incorrect data. Abort.
					continue;
				}
				
				// Create the new sensor.
				Sensor sensor(std::stoi(sensor_data[0]), config);
				
				// TODO: allow sensors to read the current conditions in the room (temp, humidity,
				// etc.).
				
				// Add sensor to the node.
				// FIXME: nodes addressing.
				nodes.at(std::stoi(actuator_data[1])).addSensor(sensor);
				
				sensors.push_back(sensor);
			}
		}
		
		if (!actuators.empty()) {
			// Extract the actuator IDs.
			std::vector<std::string> actuator_ids;
			split_string(actuators, ',', actuator_ids);	
			int actuator_count = actuator_ids.size();
			
			// Create the actuators.
			for (int i = 0; i < actuator_count; ++i) {
				// Split the sensor section into its ID and the node it is to be connected to.
				std::vector<std::string> actuator_data;
				split_string(actuator_ids.at(i), actuator_data);
				if (actuator_data.size() != 2) {
					// Incorrect data. Abort.
					continue;
				}
				
				// Create the new actuator.
				Actuator actuator(std::stoi(actuator_ids.at(i)), config);
				
				// Add actuator to the node.
				// FIXME: nodes addressing.
				nodes.at(std::stoi(actuator_data[1])).addActuator(actuator);
				
				actuators.push_back(actuator);
			}
		}
	}
	
	// Create links between the devices.
	
	
	/* switch(type) {
		case DEVICE_TYPE_NODE:
			
			
			Node node(uint32_t id, config);
			if (node.) {
				BME280 bme280(
				node.addI2C(bme280);
			
			break;
		default:
			//
	}; */
		
	// Start the Node instance.
	
}
