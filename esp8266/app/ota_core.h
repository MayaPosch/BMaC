/*
	ota_core.h - Header for the OtaCore class.
	
	Revision 1
	
	Features:
			- Declares the basic class needed for ESP8266 OTA functionality.
			- Allows for additional modules to be registered.
			
	2017/03/13, Maya Posch <posch@synyx.de>
	2017/11/15, Maya Posch <posch@synyx.de>
*/


#pragma once
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
	static uint32 active_mods;
	static String location;
	static String version;
	static int sclPin;
	static int sdaPin;
	static bool i2c_active;

	static void otaUpdate();
	static void otaUpdate_CallBack(rBootHttpUpdate& update, bool result);
	static void startMqttClient();
	static void checkMQTTDisconnect(TcpClient& client, bool flag);
	static void connectOk(IPAddress ip, IPAddress mask, IPAddress gateway);
	static void connectFail(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason);
	static void onMqttReceived(String topic, String message);
	
public:
	static bool init(onInitCallback cb);
	static bool registerTopic(String topic, topicCallback cb);
	static bool deregisterTopic(String topic);
	static bool publish(String topic, String message, int qos = 1);
	static void log(int level, String msg);
	static String getMAC() { return OtaCore::MAC; }
	static String getLocation() { return OtaCore::location; }
	static bool starti2c();
};


#endif
