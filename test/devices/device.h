/*
	device.h - Header file for the Device class.
	
	Revision 0
	
	Notes:
			- 
			
	2018/09/18, Maya Posch
*/


#ifndef DEVICE_H
#define DEVICE_H


#include "config.h"
#include "types.h"


class Device {
	std::shared_ptr<RoomState> roomState;
	Connection connType;
	std::string device;
	int spi_cs;
	int i2c_address;
	int uart_baud;		// UART baud rate.
	int uart_dev;		// UART peripheral (0, 1, etc.)
	
public:
	Device(std::string id, Config &config, std::shared_ptr<RoomState> rs);
	/* uint8_t readRegister8(int reg);
	void writeRegister8(int reg, uint8_t data); */
	Connection connectionType() { return connType; }
};


#endif
