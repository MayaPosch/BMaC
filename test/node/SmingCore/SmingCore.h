

#ifndef SMINGCORE_H
#define SMINGCORE_H

#include <cstdint>
#include <cstdio>
#include "wiring/WString.h"
#include "wiring/WVector.h"
#include "wiring/WHashMap.h"
#include "FileSystem.h"

using namespace std;



// --- HardwareSerial.h
#define UART_ID_0 0 ///< ID of UART 0
#define UART_ID_1 1 ///< ID of UART 1

class HardwareSerial {
	const int uart;
	
public:
	HardwareSerial(const int uartPort) { uart = uartPort; }
	void begin() { }
	void systemDebugOutput(bool enable) { }
	void end() { }
	size_t printf(const char *fmt, ...) { printf(
	void setCallback();
	void write();
	void readBytes();
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


// --- Wire
class Wire {
	//
	
public:
	void pins();
	void begin();
}


// --- SPI
class SPI {
	//
	
public:
	void begin();
};


// --- Delay
delayMicroseconds();
delay();


// --- GPIO
pinMode();


// --- System
class System {
	//
	
public:
	void restart() { }
};

#endif
