/*
	sensor.h - Header file for the Sensor base class.
	
	Revision 0
	
	Notes:
			- 
			
	2018/09/18, Maya Posch
*/

#ifndef SENSOR_H
#define SENSOR_H


#include "config.h"
#include "types.h"


class Sensor {
	Connection connType;
	std::string device;
	int spi_cs;
	int i2c_address;
	int uart_baud;		// UART baud rate.
	int uart_dev;		// UART peripheral (0, 1, etc.)
	
	
public:
	Sensor(int id, Config &config);
	
	Connection connectionType() { return connType; }
};

#endif
