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
#include 'device.h"

#include "sensor.h"
#include "actuator.h"

#include <string>
#include <vector>


class Node {
	std::string mac;
	Device* uart0;
	std::vector<Sensor> sensors;
	std::vector<Actuator> actuators;
	
public:
	Node(std::string id, Config &config);
	bool addSensor(Sensor &sensor);
	bool addActuator(Actuator &actuator);
	bool addI2CDevice(Sensor &sensor, int address);
	bool addI2CDevice(Actuator &actuator, int address);
	bool addSPIDevice(Sensor &sensor, int cs_pin);
	bool addSPIDevice(Actuator &actuator, int cs_pin);
	bool addUARTDevice(Sensor &sensor, int uart);
	bool addUARTDevice(Actuator &actuator, int uart);
	
	bool registerUartCb(std::string cb);
	bool writeUart(std::string bytes);
	std::string readUart();
	bool writeSPI(std::string bytes);
	std::string readSPI();
	bool writeI2C(std::string bytes);
	std::string readI2C();
};

#endif