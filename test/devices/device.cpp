/*
	device.cpp - Source file for the Device class.
	
	Revision 0
	
	Notes:
			- 
			
	2018/09/18, Maya Posch
*/


#include "device.h"


// --- CONSTRUCTOR ---
Device::Device(std::string id, Config &config, std::shared_ptr<RoomState> rs) : 
																roomState(rs),
																spi_cs(0) {
	// Read out the details for this device.
	std::string cat = "Device_" + id;
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


// --- WRITE ---
bool Device::write(std::string bytes) {
	//
	
	return true;
}


// --- READ ---
// Check the INI file for the device to see what we should be returning. Get the data from the
// room status object and return it in the appropriate format.
std::string Device::read() {
	switch (connType) {
		case CONN_SPI:
			// TODO: implement.
			return std::string();
			break;
		case CONN_I2C:
			// 
			
			break;
		case CONN_UART:
			// 
			
			break;
		default:
			// Error.
			return string();
	};
}
