

#ifndef SMINGCORE_H
#define SMINGCORE_H

#include <cstdint>
#include <cstdio>
#include "wiring/WString.h"
#include "wiring/WVector.h"
#include "wiring/WHashMap.h"
#include "FileSystem.h"
#include "WiringFrameworkDependencies.h"
#include "Stream.h"
#include "Delegate.h"

using namespace std;



// --- HardwareSerial.h
#define UART_ID_0 0 ///< ID of UART 0
#define UART_ID_1 1 ///< ID of UART 1

typedef Delegate<void(Stream& source, char arrivedChar, uint16_t availableCharsCount)> StreamDataReceivedDelegate;

class HardwareSerial {
	const int uart;
	uint32_t baud;
	StreamDataReceivedDelegate HWSDelegate = nullptr;
	
public:
	HardwareSerial(const int uartPort) { uart = uartPort; }
	void begin(uint32_t baud = 9600) { }
	void systemDebugOutput(bool enable) { }
	void end() { }
	size_t printf(const char *fmt, ...);
	void print(String str);
	void println(String str);
	void setCallback(StreamDataReceivedDelegate dataReceivedDelegate);
	static void dataReceivedCallback();
	size_t write(const uint8_t* buffer, size_t size);
	size_t readBytes(char *buffer, size_t length);
};

extern HardwareSerial Serial; // Default UART


// --- rboot
int rboot_get_current_rom() { return 0; }
void rboot_set_current_rom();
rBootHttpUpdate();


// --- SPIFFS
spiffs_mount_manual() { }


// --- Network
// STATION CLASS
class StationClass {
	String mac;
	bool enable;
	
public:
	void enable(bool enabled, bool save);
	bool config(const String& ssid, const String& password, bool autoConnectOnStartup /* = true*/,
						  bool save /* = true */);
	bool connect();
	String getMAC() { return mac; }
	
	static int handle;
};

extern StationClass WifiStation;


// ACCESS POINT CLASS
class AccessPointClass {
	bool enable;
	
public:
	void enable(bool en) { enable = en; }
};

extern AccessPointClass WifiAccessPoint;


class WifiEvents {
	//
	
public:
	//
};


// --- MQTT
class MqttClient {
	//
	
public:
	//
};


// --- Types
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

/* typedef string String;
class Vector {
	//
	
public:
	//
}; */

/* class HashMap {
	//
	
public:
	HashMap(); // template, key and value type.
}; */



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
	std::string buffer;
	
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
}

extern TwoWire Wire ;


// --- SPI
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
delayMicroseconds();
delay();


// --- GPIO
pinMode() { }
digitalWrite() { }


// --- ADC
analogRead();


// --- System
class System {
	//
	
public:
	void restart() { }
};

#endif
