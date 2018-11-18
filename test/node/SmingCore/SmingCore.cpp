
#include "SmingCore.h"

#include <nymph.h>

#include <iostream>


// NymphRPC loging function.
void logFunction(int level, string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


HardwareSerial(const int uartPort) { uart = uartPort; }
	void begin() { }
	void systemDebugOutput(bool enable) { }
	void end() { }
	size_t printf(const char *fmt, ...) { printf(
	void setCallback();
	void write();
	void readBytes();
};


// STATION CLASS
StationClass WifiStation;

void StationClass::enable(bool enabled, bool save) {
	this->enabled = enabled;
}


bool StationClass::config(const String& ssid, const String& password, bool autoConnectOnStartup /* = true*/,
						  bool save /* = true */) {
	// Nothing to do.
	
	return true;
}


bool StationClass::connect() {
	// Connect to the NymphRPC server. This gets us the commands we can call on it. 
	// The communication lines we need:
	// 1. Requesting data from remote UART/SPI/I2C device.
	//		=> Call function on remote server.
	// 2. Data received from the UART.
	//		=> Callback provided to the remote UART class.
	//
	// In this initial connection, we call the 'getNewMac' remote function to get the MAC string for
	// this firmware node instance. This gets us the MAC address which we use as a unique 
	// identifier.
	// Initialise the remote client instance.
	long timeout = 5000; // 5 seconds.
	NymphRemoteServer::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Connect to the remote server. We expect to find it via the loopback interface (localhost).
	int handle;
	std::string result;
	if (!NymphRemoteServer::connect("localhost", 4004, handle, 0, result)) {
		cout << "Connecting to remote server failed: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	// Send message and wait for response.
	vector<NymphType*> values;
	values.push_back(new NymphString("Hello World!"));
	NymphType* returnValue = 0;
	if (!NymphRemoteServer::callMethod(handle, "helloFunction", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	if (returnValue->type() != NYMPH_STRING) {
		std::cout << "Return value wasn't a string. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	std::string macStr = ((NymphString*) returnValue)->getValue();
	mac = String(macStr.data(), macStr.length());
	
	return true;
}
	
String StationClass::getMAC() { return mac; }
	
	
// ACCESS POINT CLASS
AccessPointClass WifiAccessPoint;

void AccessPointClass::enable(bool enabled, bool save) {
	// Nothing to do.
}
