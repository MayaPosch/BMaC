

#ifndef SMINGCORE_H
#define SMINGCORE_H

#include <cstdint>
#include <cstdio>
#include <string>
#include "wiring/WString.h"
#include "wiring/WVector.h"
#include "wiring/WHashMap.h"
#include "FileSystem.h"
//#include "WiringFrameworkDependencies.h"
#include "Stream.h"
#include "Delegate.h"
#include "Network/MqttClient.h"

using namespace std;



// --- HardwareSerial.h
#define UART_ID_0 0 ///< ID of UART 0
#define UART_ID_1 1 ///< ID of UART 1

typedef Delegate<void(Stream& source, char arrivedChar, uint16_t availableCharsCount)> StreamDataReceivedDelegate;

class HardwareSerial {
	int uart;
	uint32_t baud;
	StreamDataReceivedDelegate HWSDelegate = nullptr;
	std::string rxBuffer;
	
public:
	HardwareSerial(const int uartPort);
	void begin(uint32_t baud = 9600);
	void systemDebugOutput(bool enable);
	void end();
	size_t printf(const char *fmt, ...);
	void print(String str);
	void println(String str);
	void setCallback(StreamDataReceivedDelegate dataReceivedDelegate);
	void dataReceivedCallback(NymphMessage* msg, void* data);
	size_t write(const uint8_t* buffer, size_t size);
	size_t readBytes(char *buffer, size_t length);
};

extern HardwareSerial Serial; // Default UART


// --- rboot
int rboot_get_current_rom() { return 0; }
void rboot_set_current_rom();

// TODO: implement when testing OTA updates.
class rBootHttpUpdate;
typedef Delegate<void(rBootHttpUpdate& client, bool result)> OtaUpdateDelegate;
class rBootHttpUpdate {
	//

public:
	void addItem(int offset, String firmwareFileUrl) { }
	void setCallback(OtaUpdateDelegate reqUpdateDelegate) { }
	void start() { }
};


// --- SPIFFS
void spiffs_mount_manual() { }


// --- Network
// STATION CLASS
class StationClass {
	String mac;
	bool enabled;
	
public:
	void enable(bool enable, bool save);
	bool config(const String& ssid, const String& password, bool autoConnectOnStartup /* = true*/,
						  bool save /* = true */);
	bool connect();
	String getMAC() { return mac; }
	
	static int handle;
};

extern StationClass WifiStation;


// ACCESS POINT CLASS
class AccessPointClass {
	bool enabled;
	
public:
	void enable(bool en) { enabled = en; }
};

extern AccessPointClass WifiAccessPoint;


// WIFI EVENTS
class IPAddress {
	//
public:
	String toString() { return "192.168.0.32"; }
};

typedef Delegate<void(uint8_t[6], uint8_t)> AccessPointDisconnectDelegate;
typedef Delegate<void(IPAddress, IPAddress, IPAddress)> StationGotIPDelegate;
class WifiEvents {
	//
	
public:
	void onStationGotIP(StationGotIPDelegate delegateFunction) {
		// Immediately call the callback.
		IPAddress ip;
		delegateFunction(ip, ip, ip);
	}
	
	void onStationDisconnect(AccessPointDisconnectDelegate delegateFunction) {
		//
	}
};


// --- Types
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;


// --- File
/* typedef enum {
	eSO_FileStart = SPIFFS_SEEK_SET,  ///< Start of file
	eSO_CurrentPos = SPIFFS_SEEK_CUR, ///< Current position in file
	eSO_FileEnd = SPIFFS_SEEK_END	 ///< End of file
} SeekOriginFlags;

file_t fileOpen(const String& name, FileOpenFlags flags);
void fileClose(file_t file);
size_t fileWrite(file_t file, const void* data, size_t size);
size_t fileRead(file_t file, void* data, size_t size);
int fileSeek(file_t file, int offset, SeekOriginFlags origin);
int32_t fileTell(file_t file);
bool fileExist(const String& name);
int fileGetContent(const String& fileName, char* buffer, int bufSize); */


// --- WDT
class WDT {
	//
	
public:
	void alive() { }
};


// --- Wire (I2C)
class TwoWire {
	uint8_t rxBufferIndex;
	std::string buffer;
	int i2cAddress;
	
public:
	void pins(int sda, int scl);
	void begin();
	void beginTransmission(int address);
	size_t write(uint8_t data);
	size_t write(int data);
	size_t endTransmission();
	size_t requestFrom(int address, int length);
	int available();
	int read();
};

extern TwoWire Wire;


// --- SPI
class SPISettings {
	//
public:
	//
};

class SPI {
	//
	
public:
	void begin();
	void end();
	void beginTransaction(SPISettings mySettings);
	void endTransaction();
	void transfer(uint8* buffer, size_t numberBytes);
};


// --- Delay
// TODO: implement once timing becomes an issue in the simulation.
void delayMicroseconds(uint32_t time) { }
void delay(uint32_t time) { }


// --- GPIO
// TODO
void pinMode(uint16_t pin, uint8_t mode) { }
void digitalWrite(uint16_t pin, uint8_t val) { }
uint8_t digitalRead(uint16_t pin) { return 1; }


// --- ADC
uint16_t analogRead(uint16_t pin) { return 1000; }


// --- System
class System {
	//
	
public:
	void restart() { }
};

#endif
