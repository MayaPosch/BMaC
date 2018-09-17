/*
	ota_core.h - Header for the OtaCore class.
	
	Revision 2
	
	Features:
			- Declares the basic class needed for ESP8266 OTA functionality.
			- Allows for additional modules to be registered.
			
	2017/03/13, Maya Posch
	2017/11/15, Maya Posch
	2018/08/28, Maya Posch
*/


#ifndef OTA_CORE_H
#define OTA_CORE_H


#include <user_config.h>
#include <SmingCore/SmingCore.h>


enum {
	LOG_ERROR = 0,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
	LOG_TRACE,
	LOG_XTRACE
};


enum ESP8266_pins {
	ESP8266_gpio00 = 0x00001,	// Flash
	ESP8266_gpio01 = 0x00002, 	// TXD 0
	ESP8266_gpio02 = 0x00004,	// TXD 1
	ESP8266_gpio03 = 0x00008,	// RXD 0
	ESP8266_gpio04 = 0x00010,	// 
	ESP8266_gpio05 = 0x00020,	// 
	ESP8266_gpio09 = 0x00040,	// SDD2 (QDIO Flash)
	ESP8266_gpio10 = 0x00080,	// SDD3 (QDIO Flash)
	ESP8266_gpio12 = 0x00100,	// HMISO (SDO)
	ESP8266_gpio13 = 0x00200,	// HMOSI (SDI)
	ESP8266_gpio14 = 0x00400,	// SCK
	ESP8266_gpio15 = 0x00800,	// HCS
	ESP8266_gpio16 = 0x01000,	// User, Wake
	ESP8266_mosi = 0x02000,
	ESP8266_miso = 0x04000,
	ESP8266_sclk = 0x08000,
	ESP8266_cs = 0x10000
};


// I2C pins.
#define SCL_PIN 5 // SCL pin: GPIO5 ('D1' on NodeMCU)
#define SDA_PIN 4 // SDA pin: GPIO4 ('D2' on NodeMCU)


// Function pointers.
typedef void (*topicCallback)(String);
typedef void (*onInitCallback)();


class OtaCore {
	static Timer procTimer;
	static rBootHttpUpdate* otaUpdater;
	static MqttClient* mqtt;
	static String MAC;
	static HashMap<String, topicCallback>* topicCallbacks;
	static HardwareSerial Serial1;
	static String location;
	static String version;
	static int sclPin;
	static int sdaPin;
	static bool i2c_active;
	static bool spi_active;
	static uint32 esp8266_pins;

	static void otaUpdate();
	static void otaUpdate_CallBack(rBootHttpUpdate& update, bool result);
	static void startMqttClient();
	static void checkMQTTDisconnect(TcpClient& client, bool flag);
	static void connectOk(IPAddress ip, IPAddress mask, IPAddress gateway);
	static void connectFail(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason);
	static void onMqttReceived(String topic, String message);
	static void updateModules(uint32 input);
	static bool mapGpioToBit(int pin, ESP8266_pins &addr);
	
public:
	static bool init(onInitCallback cb);
	static bool registerTopic(String topic, topicCallback cb);
	static bool deregisterTopic(String topic);
	static bool publish(String topic, String message, int qos = 1);
	static void log(int level, String msg);
	static String getMAC() { return OtaCore::MAC; }
	static String getLocation() { return OtaCore::location; }
	static bool starti2c();
	static bool startSPI();
	static bool claimPin(ESP8266_pins pin);
	static bool claimPin(int pin);
	static bool releasePin(ESP8266_pins pin);
	static bool releasePin(int pin);
};


#endif
