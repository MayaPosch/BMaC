/*
	node.cpp - Node class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#include "node.h"
#include "nodes.h"

#include <cstdlib>
#include <utility.


// --- CONSTRUCTOR ---
Node::Node(std::string id, Config &config) : uart0_active(false) {
	// Read out the MAC address for this node, as well as the sensors and actuators connected
	// to this node.
	std::string node_cat = "Node_" + id;
	mac = config.getValue<std::string>(node_cat + ".mac", "");
	
	// Launch a new node instance. These run as external processes since the firmware has its
	// own main function.
	// First register this new Node with the Nodes class.
	Nodes::addNode(mac, this);
	std::system("esp8266");
	
	// The new node should now be launching and connect to the simulation's NymphRPC server.
	// We have registered the required functions at this point already.
	// => SPI/I2C: the remote node call will be directed to the proper instance.
	// => UART: remote node calls are redirected to the UART instance.
	//			Generated events are passed to the remote nodes using the callback.
	
	
};


// --- ADD DEVICE ---
bool addDevice(Device &&device) {
	// Check the device for the interface it's on (SPI, I2C or UART) and add it to the respective
	// interface.
	switch (device.connectionType()) {
		case CONN_SPI:
			spi.insert(std::pair<int, Device>(device.spiCS(), std::move(device)));
			break;
		case CONN_I2C:
			i2c.insert(std::pair<int, Device>(device.i2cAddress(), std::move(device)));
			break;
		case CONN_UART:
			uart0 = std::move(device);
			uart0_active = true;
			break;
		default:
			// Error.
			break;
	}
}


// --- REGISTER UART CB ---
/* bool registerUartCb(std::string cb) {
	// TODO: implement, as well as CB trigger.
	
} */


// --- WRITE UART ---
bool writeUart(std::string bytes) {
	// We write the provided bytes on the single UART the ESP8266 has, assuming that a device has
	// been connected.
	if (!uart0_active) { return false; }
	
	uart0.write(bytes);
	
	return true;
}


// --- READ UART ---
std::string readUart() {
	if (!uart0_active) { return false; }
	
	uart0.read();
}


// --- WRITE SPI ---
bool writeSPI(std::string bytes) {
	// TODO: SPI CS handling.
	if (spi.count() == 1) {
		spi[0].write(bytes);
	}
	else {
		return false; 
	}
	
	return true;
}


// --- READ SPI ---
std::string readSPI() {
	// TODO: SPI CS handling.
	if (spi.count() == 1) {
		return spi[0].read();
	}
	else {
		return string();
	}
}


// --- WRITE I2C ---
bool writeI2C(std::string bytes) {
	// The first four bytes (int) are the device address. Extract it and use it to find the device.
	// Send the remaining data to the Device instance.
	int address = std::stoi(bytes.substr(0, 4));
	bytes.erase(0, 4);
	if ((i2c.find(address)) != i2c.end()) { return false; }
	
	i2c[address].write(bytes);
	return true;
}


// --- READ I2C ---
std::string readI2C() {
	int address = std::stoi(bytes.substr(0, 4));
	bytes.erase(0, 4);
	if (i2c.count(address)) { return string(); }
	
	return i2c[address].read(bytes);
}
