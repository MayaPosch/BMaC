
#include "SmingCore.h"

#include <iostream>
#include <cstdio>
#include <cstdarg>

using namespace std;


// Static initialisations.
int StationClass::handle;


// NymphRPC logging function.
void logFunction(int level, string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


// --- Debug
void debugf(const char *fmt, ...) { 
	va_list ap;
    va_start(ap, fmt);
    int written = vfprintf(stdout, fmt, ap);
    va_end(ap);
}


// HARDWARE SERIAL
// Static initialisation.
StreamDataReceivedDelegate HardwareSerial::HWSDelegate = nullptr;
std::string HardwareSerial::rxBuffer;
HardwareSerial Serial(0);

SerialStream::SerialStream() { }
size_t SerialStream::write(uint8_t) { return 1; }
int SerialStream::available() { return 0; }
int SerialStream::read() { return 0; }
void SerialStream::flush() { }
int SerialStream::peek() { return 0; }

HardwareSerial::HardwareSerial(const int uartPort) { 
	uart = uartPort; 
}


void HardwareSerial::begin(uint32_t baud/* = 9600*/) { 
	this->baud = baud;
}


void HardwareSerial::systemDebugOutput(bool enable) { }
void HardwareSerial::end() { }
size_t HardwareSerial::printf(const char *fmt, ...) { 
	va_list ap;
    va_start(ap, fmt);
    int written = vfprintf(stdout, fmt, ap);
    va_end(ap);
	
	return written;
}


void HardwareSerial::print(String str) {
	std::cout << str.c_str();
}


void HardwareSerial::println(String str) {
	std::cout << str.c_str() << std::endl;
}


void HardwareSerial::println(const char* str) {
	std::cout << str << std::endl;
}


void HardwareSerial::println(int16_t ch) {
	std::cout << std::hex << ch << std::endl;
}


void HardwareSerial::setCallback(StreamDataReceivedDelegate dataReceivedDelegate) {
	HWSDelegate = dataReceivedDelegate;
	
	// Call the remote method.
	/* vector<NymphType*> values;
	values.push_back(new NymphString(WifiStation.getMAC()));
	values.push_back(new NymphString("dataReceivedCallback"));
	NymphType* returnValue = 0;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "registerUartCb", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	if (returnValue->type() != NYMPH_BOOL) {
		std::cout << "Return value wasn't a boolean. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return false;
	} */
}


void HardwareSerial::dataReceivedCallback(NymphMessage* msg, void* data) {
	// Get the data from the message.
	rxBuffer = ((NymphString*) msg->parameters()[0])->getValue();
	
	// Call the callback method, if one has been registered.
	// Loop through the received data, sending one character at a time on the registered callback.
	SerialStream stream;
	int length = rxBuffer.length();
	int i = 0;
	//for (int i = 0; i < length; ++i) {
		HWSDelegate(stream, rxBuffer[i], length - i);
	//}
}


size_t HardwareSerial::write(const uint8_t* buffer, size_t size) {
	// Call the remote method.
	vector<NymphType*> values;
	values.push_back(new NymphString(WifiStation.getMAC().c_str()));
	values.push_back(new NymphString(std::string((const char*) buffer, size)));
	NymphType* returnValue = 0;
	std::string result;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "writeUart", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return 0;
	}
	
	if (returnValue->type() != NYMPH_BOOL) {
		std::cout << "Return value wasn't a boolean. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return 0;
	}
	
	return size;
}


size_t HardwareSerial::write(uint8_t oneChar) {
	const uint8_t constChar = oneChar;
	return write(&constChar, 1);
}


size_t HardwareSerial::readBytes(char* buffer, size_t length) {
	// Call the remote function.
	/* vector<NymphType*> values;
	values.push_back(new NymphString(WifiStation.getMAC().c_str()));
	NymphType* returnValue = 0;
	std::string result;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "readUart", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	if (returnValue->type() != NYMPH_STRING) {
		std::cout << "Return value wasn't a string. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	// Update buffer, return bytes read.
	std::string bytes = ((NymphString*) returnValue)->getValue(); */
	
	buffer = rxBuffer.data();
	return rxBuffer.length();
}


// RBOOT
int rboot_get_current_rom() { return 0; }
void rboot_set_current_rom(int slot) { }
rboot_config rboot_get_config() {
	rboot_config cfg;
	cfg.current_rom = 0;
	cfg.roms[0] = 0x1000;
	cfg.roms[1] = 0x3000;
	return cfg;
}

// TODO: implement when testing OTA updates.
void rBootHttpUpdate::addItem(int offset, String firmwareFileUrl) { }
void rBootHttpUpdate::setCallback(OtaUpdateDelegate reqUpdateDelegate) { }
void rBootHttpUpdate::start() { }


// --- SPIFFS
void spiffs_mount_manual(u32_t offset, int count) { }


// STATION CLASS
StationClass WifiStation;

void StationClass::enable(bool enable) { enabled = enable; }
void StationClass::enable(bool enable, bool save) { enabled = enable; }
String StationClass::getMAC() { return mac; }

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
	std::string result;
	if (!NymphRemoteServer::connect("localhost", 4004, StationClass::handle, 0, result)) {
		cout << "Connecting to remote server failed: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	// Send message and wait for response.
	vector<NymphType*> values;
	NymphType* returnValue = 0;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "getNewMac", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	if (returnValue->type() != NYMPH_STRING) {
		std::cout << "Return value wasn't a string. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return false;
	}
	
	std::string macStr = ((NymphString*) returnValue)->getValue();
	mac = String(macStr.data(), macStr.length());
	
	delete returnValue;
	returnValue = 0;
	
	// Set the serial interface callback.
	NymphRemoteServer::registerCallback("serialRxCallback", HardwareSerial::dataReceivedCallback, 0);
	
	return true;
}
	
	
// ACCESS POINT CLASS
AccessPointClass WifiAccessPoint;

void AccessPointClass::enable(bool enable, bool save) {
	enabled = enable;
}

void AccessPointClass::enable(bool enable) {
	enabled = enable;
}


// WIFI EVENTS CLASS
WifiEventsClass WifiEvents;

String IPAddress::toString() { return "192.168.0.32"; }

void WifiEventsClass::onStationGotIP(StationGotIPDelegate delegateFunction) {
	// Immediately call the callback.
	IPAddress ip;
	delegateFunction(ip, ip, ip);
}

void WifiEventsClass::onStationDisconnect(StationDisconnectDelegate delegateFunction) {
	//
}


// WDT
WDTClass WDT;

void WDTClass::alive() { }

void yield() { }


// --- Shift
uint16_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t count /*= 8*/, uint8_t delayTime /*= 1*/) { return 1; }
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint16_t value, uint8_t count /*= 8*/, uint8_t delayTime /*= 1*/) { }
	
	
// SPI
void SPIBase::begin() { }
void SPIBase::end() { }
void SPIBase::beginTransaction(SPISettings mySettings) { }
void SPIBase::endTransaction() { }
void SPIBase::transfer(uint8* buffer, size_t numberBytes) {
	// Call the remote method.
	vector<NymphType*> values;
	values.push_back(new NymphString(WifiStation.getMAC().c_str()));
	values.push_back(new NymphString(std::string((char*) buffer, numberBytes)));
	NymphType* returnValue = 0;
	std::string result;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "writeSPI", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
	
	if (returnValue->type() != NYMPH_BOOL) {
		std::cout << "Return value wasn't a boolean. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
}

unsigned short SPIBase::transfer16(unsigned short val) {
	// TODO
}

unsigned char SPIBase::transfer(unsigned char val) {
	// TODO
}

SPIBase SPI;


// I2C
void TwoWire::pins(int sda, int scl) { }
void TwoWire::begin() { }
void TwoWire::beginTransmission(int address) { i2cAddress = address; }
size_t TwoWire::write(uint8_t data) {
	// Call remote method.
	vector<NymphType*> values;
	values.push_back(new NymphString(WifiStation.getMAC().c_str()));
	values.push_back(new NymphSint32(i2cAddress));
	values.push_back(new NymphString(std::to_string(data)));
	NymphType* returnValue = 0;
	std::string result;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "writeI2C", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return 0;
	}
	
	if (returnValue->type() != NYMPH_BOOL) {
		std::cout << "Return value wasn't a boolean. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return 0;
	}
	
	return 1;
}


size_t TwoWire::write(int data) {
	// Call remote method.
	vector<NymphType*> values;
	values.push_back(new NymphString(WifiStation.getMAC().c_str()));
	values.push_back(new NymphSint32(i2cAddress));
	values.push_back(new NymphString(std::to_string(data)));
	NymphType* returnValue = 0;
	std::string result;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "writeI2C", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return 0;
	}
	
	if (returnValue->type() != NYMPH_BOOL) {
		std::cout << "Return value wasn't a boolean. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		return 0;
	}
	
	return 1;
}


size_t TwoWire::endTransmission() { return 0; }
size_t TwoWire::requestFrom(int address, int length) {
	// First write the address.
	write(address);
	
	// Call remote method. This will read the string into a local buffer.
	vector<NymphType*> values;
	values.push_back(new NymphString(WifiStation.getMAC().c_str()));
	values.push_back(new NymphSint32(address)); // Number of bytes to read.
	values.push_back(new NymphSint32(length)); // Number of bytes to read.
	NymphType* returnValue = 0;
	std::string result;
	if (!NymphRemoteServer::callMethod(StationClass::handle, "readI2C", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		exit(1);
	}
	
	if (returnValue->type() != NYMPH_STRING) {
		std::cout << "Return value wasn't a string. Type: " << returnValue->type() << std::endl;
		NymphRemoteServer::disconnect(StationClass::handle, result);
		NymphRemoteServer::shutdown();
		exit(1);
	}
	
	rxBufferIndex = 0; // reset buffer index.
	buffer = ((NymphString*) returnValue)->getValue();
	return buffer.size();
}


int TwoWire::available() {
	return buffer.length() - rxBufferIndex;
}


int TwoWire::read() {
	int value = -1;
	if (rxBufferIndex < buffer.length()) {
		value = buffer.at(rxBufferIndex);
		++rxBufferIndex;
	}
	
	return value;
}

TwoWire Wire;

// --- System
String system_get_sdk_version() { return "SIM_0.1"; }
int system_get_free_heap_size() { return 20000; }
int system_get_cpu_freq() { return 1200000; }
int system_get_chip_id() { return 42; }
int spi_flash_get_id() { return 42; }

void SystemClass::restart() { }

SystemClass System;

// DELAY
//void delayMicroseconds(uint32_t time) { }
//void delay(uint32_t time) { }


// --- GPIO
// TODO
void pinMode(uint16_t pin, uint8_t mode) { }
void digitalWrite(uint16_t pin, uint8_t val) { }
uint8_t digitalRead(uint16_t pin) { return 1; }


// --- INTERRUPTS
void attachInterrupt(uint8_t pin, InterruptCallback callback, uint8_t mode) {
	// TODO.
}

void detachInterrupt(uint8_t pin) {
	// TODO.
}

void cli() { }
void sei() { }


// --- ADC
uint16_t analogRead(uint16_t pin) { return 1000; }


// MAIN
int main() {
	// Start the firmware image.
	init();
	
	return 0;
}
