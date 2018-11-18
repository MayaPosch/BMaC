/*
	device_types.cpp - Device types header.
	
	Revision 0
	
	Notes:
			- Gathers all the device types into a single header.
		
	2018/09/18, Maya Posch
*/


// Devices.
#include "node.h"
#include "coffee_machine.h"
#include "ac.h"
#include "plant.h"
#include "mh-z19.h"
#include "bme280.h"
#include "apa102.h"

// Interfaces.
#include "uart.h"
#include "i2c.h"
#include "spi.h"



enum DeviceTypes {
	DEVICE_TYPE_NODE = 1,
	DEVICE_TYPE_COFFEE_MACHINE,
	DEVICE_TYPE_AC,
	DEVICE_TYPE_BME280,
	DEVICE_TYPE_MH_Z19,
	DEVICE_TYPE_
	DEVICE_TYPE_
};
