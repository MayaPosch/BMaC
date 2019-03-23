

#ifndef SMINGCORE_H
#define SMINGCORE_H


#include <cstdint>
#include <cstdio>
#include <string>
#include <iostream>


// --- Types
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef uint32_t u32_t;
typedef uint8_t byte;


#include "wiring/WString.h"
#include "wiring/WVector.h"
#include "wiring/WHashMap.h"
#include "FileSystem.h"
#include "wiring/Stream.h"
#include "Delegate.h"
#include "Network/MqttClient.h"
#include "Timer.h"
#include "WConstants.h"
#include "Clock.h"
#include "HardwarePWM.h"
#include "wiring/BitManipulations.h"

#include <nymph/nymph.h>


// --- HardwareSerial.h
#define UART_ID_0 0 ///< ID of UART 0
#define UART_ID_1 1 ///< ID of UART 1
#define SERIAL_BAUD_RATE 115200

#define SMING_DEPRECATED

typedef Delegate<void(Stream& source, char arrivedChar, uint16_t availableCharsCount)> StreamDataReceivedDelegate;

class SerialStream : public Stream {
	//
	
public:
	SerialStream();
	size_t write(uint8_t);
	int available();
	int read();
	void flush();
	int peek();
};

class HardwareSerial {
	int uart;
	uint32_t baud;
	static StreamDataReceivedDelegate HWSDelegate;
	static std::string rxBuffer;
	
public:
	HardwareSerial(const int uartPort);
	void begin(uint32_t baud = 9600);
	void systemDebugOutput(bool enable);
	void end();
	size_t printf(const char *fmt, ...);
	void print(String str);
	void println(String str);
	void println(const char* str);
	void println(int16_t ch);
	void setCallback(StreamDataReceivedDelegate dataReceivedDelegate);
	static void dataReceivedCallback(NymphMessage* msg, void* data);
	size_t write(const uint8_t* buffer, size_t size);
	size_t write(uint8_t oneChar);
	size_t readBytes(char *buffer, size_t length);
};

extern HardwareSerial Serial; // Default UART


// --- rboot
struct rboot_config {
	uint8 current_rom;
	uint32 roms[2];
};

int rboot_get_current_rom();
void rboot_set_current_rom(int slot);
rboot_config rboot_get_config();

// TODO: implement when testing OTA updates.
class rBootHttpUpdate;
typedef Delegate<void(rBootHttpUpdate& client, bool result)> OtaUpdateDelegate;
class rBootHttpUpdate {
	//

public:
	void addItem(int offset, String firmwareFileUrl);
	void setCallback(OtaUpdateDelegate reqUpdateDelegate);
	void start();
};


// --- SPIFFS
void spiffs_mount_manual(u32_t offset, int count);


// --- Network
// STATION CLASS
class StationClass {
	String mac;
	bool enabled;
	
public:
	void enable(bool enable);
	void enable(bool enable, bool save);
	bool config(const String& ssid, const String& password, bool autoConnectOnStartup = true,
						  bool save = true);
	bool connect();
	String getMAC();
	
	static int handle;
};

extern StationClass WifiStation;


// ACCESS POINT CLASS
class AccessPointClass {
	bool enabled;
	
public:
	void enable(bool enable, bool save);
	void enable(bool enable);
};

extern AccessPointClass WifiAccessPoint;


// WIFI EVENTS
class IPAddress {
	//
public:
	String toString();
};

typedef Delegate<void(uint8_t[6], uint8_t)> AccessPointDisconnectDelegate;
typedef Delegate<void(String, uint8_t, uint8_t[6], uint8_t)> StationDisconnectDelegate;
typedef Delegate<void(IPAddress, IPAddress, IPAddress)> StationGotIPDelegate;
class WifiEventsClass {
	//
	
public:
	void onStationGotIP(StationGotIPDelegate delegateFunction);	
	void onStationDisconnect(StationDisconnectDelegate delegateFunction);
};

extern WifiEventsClass WifiEvents;


// --- Debug
void debugf(const char *fmt, ...);
void debug_e(const char *fmt, ...);


// --- WDT
class WDTClass {
	//
	
public:
	void alive();
};

extern WDTClass WDT;

void yield();


// --- Shift
uint16_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t count = 8, uint8_t delayTime = 1);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint16_t value, uint8_t count = 8, uint8_t delayTime = 1);


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

class SPISettings;
#include "SPISettings.h"
//#include "SPIBase.h"

class SPIBase { // : public SPIBase {
	//
	
public:
	SPIBase() {}
	~SPIBase() {}
	void begin();
	void end();
	void beginTransaction(SPISettings mySettings);
	void endTransaction();
	void transfer(uint8* buffer, size_t numberBytes);
	unsigned short transfer16(unsigned short val);
	unsigned char transfer(unsigned char val);
};

extern SPIBase SPI;


// --- GPIO
void pinMode(uint16_t pin, uint8_t mode);
void digitalWrite(uint16_t pin, uint8_t val);
uint8_t digitalRead(uint16_t pin);


// --- INTERRUPTS
void attachInterrupt(uint8_t pin, InterruptCallback callback, uint8_t mode);
void detachInterrupt(uint8_t pin);
void cli();
void sei();


// --- ADC
const unsigned int A0 = 42;
uint16_t analogRead(uint16_t pin);


// --- System
String system_get_sdk_version();
int system_get_free_heap_size();
int system_get_cpu_freq();
int system_get_chip_id();
int spi_flash_get_id();

class SystemClass {
	//
	
public:
	void restart();
};

extern SystemClass System;


// --- TcpClient ---
class TcpClient {
	//
	
public:
	//
};


extern void init();

#endif
