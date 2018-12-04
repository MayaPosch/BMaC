/*
	types.h - Header file for custom types.
	
	Revision 0
	
	Notes:
			- 
			
	2018/09/18, Maya Posch
*/

#ifndef SENSOR_H
#define SENSOR_H

#include <memory>
#include <thread>
#include <mutex>


enum Connection {
	CONN_NC = 0,
	CONN_SPI = 1,
	CONN_I2C = 2,
	CONN_UART = 3
};


// 
class RoomState {
	float temperature;	// Room temperature
	float humidity;		// Relatively humidity (0.00 - 100.00%)
	uint16_t pressure;	// Air pressure.
	std::mutex tmtx;
	std::mutex hmtx;
	std::mutex pmtx;
	
public:
	RoomState() : 
		temperature(0),
		humidity(0),
		pressure(1000) {
		//
	}

	float getTemperature() {
		std::lock_guard<std::mutex> lk(tmtx); 
		return temperature; 
	
	}
	
	void setTemperature(float t) {
		std::lock_guard<std::mutex> lk(tmtx); 
		temperature = t; 
	}
	
	float getHumidity() {
		std::lock_guard<std::mutex> lk(hmtx); 
		return humidity;
	}
	
	void setHumidity(float h) {
		std::lock_guard<std::mutex> lk(hmtx);
		temperature = h; 
	}	
	
	float getPressure() {
		std::lock_guard<std::mutex> lk(pmtx); 
		return pressure;
	}
	
	void setPressure(uint16_t p) {
		std::lock_guard<std::mutex> lk(pmtx);
		pressure = p;
	}
};


#endif