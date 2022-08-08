

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


/* #define __WIFI_SSID "foo"
#define __WIFI_PWD "bar"
 */

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
	void onDataReceived(StreamDataReceivedDelegate dataReceivedDelegate);
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
class rBootHttpUpdater;
typedef Delegate<void(rBootHttpUpdater& client, bool result)> OtaUpdateDelegate;
class rBootHttpUpdater {
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
/**
 * @brief Common set of reason codes to IEEE 802.11-2007
 * @note Codes at 200+ are non-standard, defined by Espressif.
 *
 * Some acronymns used here - see the full standard for more precise definitions.
 *	- SSID: Service Set Identifier (the visible name given to an Access Point)
 *	- BSSID: Basic Service Set Identifier (a MAC address physically identifying the AP)
 *	- IE: Information Element (standard piece of information carried within WiFi packets)
 *	- STA: Station (any device which supports WiFi, including APs though the term commonly refers to a client)
 *	- AP: Access Point (device to which other stations may be associated)
 *	- RSN: Robust Security Network
 *	- AUTH: Authentication (how a station proves its identity to another)
 *
 */
#define WIFI_DISCONNECT_REASON_CODES_MAP(XX)                                                                           \
	XX(UNSPECIFIED, 1, "Unspecified")                                                                                  \
	XX(AUTH_EXPIRE, 2, "AUTH expired")                                                                                 \
	XX(AUTH_LEAVE, 3, "Sending STA is leaving, or has left")                                                           \
	XX(ASSOC_EXPIRE, 4, "Disassociated: inactivity")                                                                   \
	XX(ASSOC_TOOMANY, 5, "Disassociated: too many clients)")                                                           \
	XX(NOT_AUTHED, 6, "Class 2 frame received from non-authenticated STA")                                             \
	XX(NOT_ASSOCED, 7, "Class 3 frame received from non-authenticated STA")                                            \
	XX(ASSOC_LEAVE, 8, "Disassociated: STA is leaving, or has left")                                                   \
	XX(ASSOC_NOT_AUTHED, 9, "Disassociated: STA not authenticated")                                                    \
	XX(DISASSOC_PWRCAP_BAD, 10, "Disassociated: power capability unacceptable")                                        \
	XX(DISASSOC_SUPCHAN_BAD, 11, "Disassociated: supported channels unacceptable")                                     \
	XX(IE_INVALID, 13, "Invalid IE")                                                                                   \
	XX(MIC_FAILURE, 14, "Message Integrity failure")                                                                   \
	XX(4WAY_HANDSHAKE_TIMEOUT, 15, "4-way Handshake timeout")                                                          \
	XX(GROUP_KEY_UPDATE_TIMEOUT, 16, "Group Key Handshake timeout")                                                    \
	XX(IE_IN_4WAY_DIFFERS, 17, "4-way Handshake Information Differs")                                                  \
	XX(GROUP_CIPHER_INVALID, 18, "Invalid group cypher")                                                               \
	XX(PAIRWISE_CIPHER_INVALID, 19, "Invalid pairwise cypher")                                                         \
	XX(AKMP_INVALID, 20, "Invalid AKMP")                                                                               \
	XX(UNSUPP_RSN_IE_VERSION, 21, "Unsupported RSN IE Version")                                                        \
	XX(INVALID_RSN_IE_CAP, 22, "Invalid RSN IE capabilities")                                                          \
	XX(802_1X_AUTH_FAILED, 23, "IEEE 802.1X authentication failed")                                                    \
	XX(CIPHER_SUITE_REJECTED, 24, "Cipher suite rejected (security policy)")                                           \
	XX(BEACON_TIMEOUT, 200, "Beacon Timeout")                                                                          \
	XX(NO_AP_FOUND, 201, "No AP found")                                                                                \
	XX(AUTH_FAIL, 202, "Authentication failure")                                                                       \
	XX(ASSOC_FAIL, 203, "Association failure")                                                                         \
	XX(HANDSHAKE_TIMEOUT, 204, "Handshake timeout")                                                                    \
	XX(CONNECTION_FAIL, 205, "Connection failure")

/**
 * @brief Reason codes for WiFi station disconnection
 * @see WIFI_DISCONNECT_REASON_CODES_MAP
 */
enum WifiDisconnectReason {
#define XX(tag, code, desc) WIFI_DISCONNECT_REASON_##tag = code,
	WIFI_DISCONNECT_REASON_CODES_MAP(XX)
#undef XX
};


class IpAddress {
	//
public:
	String toString();
};

typedef String MacAddress;

typedef Delegate<void(uint8_t[6], uint8_t)> AccessPointDisconnectDelegate;
typedef Delegate<void(const String&, MacAddress, WifiDisconnectReason)> StationDisconnectDelegate;
typedef Delegate<void(IpAddress, IpAddress, IpAddress)> StationGotIPDelegate;
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
