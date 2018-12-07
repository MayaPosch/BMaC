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
	
	// Try to open the INI file for the device. On success the device state is set to 'true'.
	//deviceState = devConf.load(device + ".ini");
			
	// Check the INI file for the device to see what info we should be returning. 
	// Find the 'read' entry.
	
}


// --- WRITE ---
bool Device::write(std::string bytes) {
	if (!deviceState) { return false; }
	
	// The first byte contains the register to read/write with I2C. Keep it as reference.
	if (connType == CONN_I2C && bytes.length() > 0) {
		i2c_register = bytes[0];
	}
	else if (connType == CONN_SPI) {
		// TODO.
	}
	else if (connTYPE == CONN_UART) {
		//
	}
	else { return false; }
	
	return true;
}


// --- READ ---
// Get the data from the room status object and return it in the appropriate format.
// TODO: use the length parameter.
std::string Device::read(int length) {
	if (!deviceState) { return string(); }
	
	switch (connType) {
		case CONN_SPI:
			// TODO: implement.
			return std::string();
			break;
		case CONN_I2C:
			// Get the specified values from the room state instance.
			// Here we hard code a BME280 sensor.
			// Which value we return depends on the register set.
			uint8_t zero = 0x0;
			switch (i2c_register) {
				case 0xFA: // Temperature. MSB, LSB, XLSB.
					std::string ret = std::to_string(roomState->getTemperature()); // MSB
					ret.append(std::to_string(zero); // LSB
					ret.append(std::to_string(zero); // XLSB
					return ret;
					break;
				case 0xF7: // Pressure. MSB, LSB, XLSB.
					std::string ret = std::to_string(roomState->getPressure()); // MSB
					ret.append(std::to_string(zero); // LSB
					ret.append(std::to_string(zero); // XLSB
					return ret;
					break;
				case 0xFD: // Humidity. MSB, LSB.
					std::string ret = std::to_string(roomState->getHumidity()); // MSB
					ret.append(std::to_string(zero); // LSB
					return ret;
					break;
				default:
					return std::string();
					break;
			}
			
			break;
		case CONN_UART:
			// 
			
			break;
		default:
			// Error.
			return string();
	};
}


// --- READ ---
// Overloaded function for implementations which do not use a length parameter.
std::string Device::read() {
	read(0);
}
