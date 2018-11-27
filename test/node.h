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

#include <string>
#include <vector>
#include <map>


class Node {
	std::string mac;
	bool uart0_active;
	Device uart0;
	std::map<int, Device> i2c;
	std::vector<Device> devices;
	
public:
	Node(std::string id, Config &config);
	bool addDevice(Device &&device);
	
	bool registerUartCb(std::string cb);
	bool writeUart(std::string bytes);
	std::string readUart();
	bool writeSPI(std::string bytes);
	std::string readSPI();
	bool writeI2C(std::string bytes);
	std::string readI2C();
};

#endif