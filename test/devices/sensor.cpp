/*
	sensor.cpp - Source file for the Sensor base class.
	
	Revision 0
	
	Notes:
			- 
			
	2018/09/18, Maya Posch
*/


#include "sensor.h"


// --- CONSTRUCTOR ---
Sensor::Sensor(int id, Config &config) {
	// Read out the details for this sensor.
	std::string cat = "Sensor_" + std::to_string(id);
	std::string type = config.getValue<int>(cat + ".type", "");
	if (type == "spi") {
		connType = CONN_SPI;
		spi_cs = config.getValue<int>(cat + ".cs_gpio", 0);
		device = config.getValue<std::string>(cat + ".device", "");
	}
	else if (type == "i2c") {
		connType == CONN_I2C;
		i2c_address = config.getValue<int>(cat + ".address", 0);
		device = config.getValue<std::string>(cat + ".device", "");
	}
	else if (type == "uart") {
		connType == CONN_UART;
		uart_baud = config.getValue<int>(cat + ".baud", 0);
		uart_dev = config.getValue<int>(cat + ".uart", 0);
		device = config.getValue<std::string>(cat + ".device", "");
	}
	else {
		// Error. Invalid type.
	}
	
	// 
}
