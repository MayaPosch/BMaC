/*
	node.h - Node class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#ifndef NODE_H
#define NODE_H

#include "config.h"
#include "sensor.h"
#include "actuator.h"

#include <string>
#include <vector>


class Node {
	std::string mac;
	std::vector<Sensor> sensors;
	std::vector<Actuator> actuators;
	
public:
	Node(int id, Config &config);
	bool addSensor(Sensor &sensor);
	bool addActuator(Actuator &actuator);
	bool addI2CDevice(Sensor &sensor, int address);
	bool addI2CDevice(Actuator &actuator, int address);
	bool addSPIDevice(Sensor &sensor, int cs_pin);
	bool addSPIDevice(Actuator &actuator, int cs_pin);
	bool addUARTDevice(Sensor &sensor, int uart);
	bool addUARTDevice(Actuator &actuator, int uart);
};

#endif