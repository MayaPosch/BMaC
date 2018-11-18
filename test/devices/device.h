/*
	device.h - Header file for the Device base class.
	
	Revision 0
	
	Notes:
			- 
			
	2018/09/18, Maya Posch
*/


#ifndef DEVICE_H
#define DEVICE_H


class Device {
	//
	
public:
	uint8_t readRegister8(int reg);
	void writeRegister8(int reg, uint8_t data);
};


#endif
