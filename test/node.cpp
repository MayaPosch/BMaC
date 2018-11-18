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


// --- CONSTRUCTOR ---
Node::Node(int id, Config &config) {
	// Read out the MAC address for this node, as well as the sensors and actuators connected
	// to this node.
	std::string node_cat = "Node_" + std::to_string(id);
	mac = config.getValue<std::string>(node_cat + ".mac", "");
	//std::string sensors = config.getValue<std::string>(node_cat + ".sensors", "");
	//std::string actuators = config.getValue<std::string>(node_cat + ".actuators", "");
	
	// Launch a new node instance. These run as external processes since the firmware has its
	// own main function.
	// First push the MAC for it onto the Nodes queue.
	Nodes::addMAC(mac);
	std::system("esp8266");
	
	// The new node should now be launching and connect to the simulation's NymphRPC server.
	// We have registered the required functions at this point already.
	// => SPI/I2C: the remote node call will be directed to the proper instance.
	// => UART: remote node calls are redirected to the UART instance.
	//			Generated events are passed to the remote nodes using the callback.
	
	
};


// --- ADD SENSOR ---
bool addSensor(Sensor &sensor) {
	// Check the sensor for the interface it's on (SPI, I2C or UART) and add it to the respective
	// interface.
	switch (sensor.connectionType()) {
		case CONN_SPI:
			//
			break;
		case CONN_I2C:
			//
			break;
		case CONN_UART:
			//
			break;
		default:
			// Error.
			break;
	}
}


// --- ADD ACTUATOR ---
bool addActuator(Actuator &actuator) {
	// Check the actuator for the interface it's on (SPI, I2C or UART) and add it to the respective
	// interface.
	
}


// --- ADD I2C DEVICE ---
// Add an I2C sensor to the node's I2C bus, with the provided address.
bool Node::addI2CDevice(Sensor &sensor, int address) {
	//
}


// Add an I2C actuator to the node's I2C bus, with the provided address.
bool Node::AddI2CDevice(Actuator &actuator, int address) {
	//
}


// --- ADD SPI DEVICE ---
// Add an SPI sensor to the node's SPI bus, on the provided chip-select pin.
bool Node::addSPIDevice(Sensor &sensor, int cs_pin) {
	//
}


// Add an SPI actuator to the node's SPI bus, on the provided chip-select pin.
bool Node::AddSPIDevice(Actuator &actuator, int cs_pin) {
	//
}


// --- ADD UART DEVICE ---
// Add a UART sensor to the specified UART device.
bool addUARTDevice(Sensor &sensor, int uart) {
	//
}


// Add a UART actuator to the specified UART device.
bool addUARTDevice(Actuator &actuator, int uart) {
	//
}
