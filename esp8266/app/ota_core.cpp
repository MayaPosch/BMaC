/*
	ota_core.h - Header for the OtaCore class.
	
	Revision 0
	
	Features:
			- Defines the basic class needed for ESP8266 OTA functionality.
			- Allows for additional modules to be registered.
			
	Notes:
			- 
			
	2017/03/13, Maya Posch <posch@synyx.de>
*/


#include <ota_core.h>


// Feature modules.
#include "thp_module.h"
#include "jura_module.h"
#include "juraterm_module.h"
#include "co2_module.h"
#include "motion_module.h"
#include "pwm_module.h"
#include "io_module.h"
#include "switch_module.h"


// Enums
enum {
	MOD_TEMPERATURE_HUMIDITY = 0x1,
	MOD_CO2 = 0x2,
	MOD_JURA = 0x4,
	MOD_JURATERM = 0x8,
	MOD_MOTION = 0x10,
	MOD_PWM = 0x20,
	MOD_IO = 0x40,
	MOD_SWITCH = 0x80
};


// Static initialisations.
Timer OtaCore::procTimer;
rBootHttpUpdate* OtaCore::otaUpdater = 0;
MqttClient* OtaCore::mqtt = 0;
String OtaCore::MAC;
HashMap<String, topicCallback>* OtaCore::topicCallbacks = new HashMap<String, topicCallback>();
uint32 OtaCore::active_mods = 0x0;
HardwareSerial OtaCore::Serial1(UART_ID_1); // UART 0 is 'Serial'.
String OtaCore::location;
String OtaCore::version = VERSION;
int OtaCore::sclPin = SCL_PIN; // default.
int OtaCore::sdaPin = SDA_PIN; // default.
bool OtaCore::i2c_active = false;


// Utility function to circumvent a bug in older versions of the 
// Sming FileSystem class for fileGetContent(). 
// It doesn't deal with null bytes very well, which is a problem for 
// binary files like our DER-formatted certificate and key.
String getFileContent(const String fileName) {
	file_t file = fileOpen(fileName.c_str(), eFO_ReadOnly);
	
	// Get size
	fileSeek(file, 0, eSO_FileEnd);
	int size = fileTell(file);
	if (size <= 0) 	{
		fileClose(file);
		return "";
	}
	
	fileSeek(file, 0, eSO_FileStart);
	char* buffer = new char[size + 1];
	buffer[size] = 0;
	fileRead(file, buffer, size);
	fileClose(file);
	String res(buffer, size);
	delete[] buffer;
	return res;
}


bool readIntoFileBuffer(const String filename, char* &buffer, unsigned int &size) {
	file_t file = fileOpen(filename.c_str(), eFO_ReadOnly);
	
	// Get size
	fileSeek(file, 0, eSO_FileEnd);
	size = fileTell(file);
	if (size <= 0) 	{
		fileClose(file);
		return true;
	}
	
	fileSeek(file, 0, eSO_FileStart);
	buffer = new char[size + 1];
	buffer[size] = 0;
	fileRead(file, buffer, size);
	fileClose(file);
	return true;
}


// --- INIT ---
// Initialise the static class.
bool OtaCore::init(onInitCallback cb) {
	Serial.begin(9600);

	Serial1.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial1.systemDebugOutput(true); // Debug output to serial
	
	//spiffs_mount(); // Mount file system, in order to work with files.
	
	// Mount the SpifFS manually. Automatic mounting is not
	// compatible with RBoot (yet):
	// https://github.com/SmingHub/Sming/issues/1009
	int slot = rboot_get_current_rom();
	u32_t offset;
	if (slot == 0) { offset = 0x100000; }
	else { offset = 0x300000; }
	spiffs_mount_manual(offset, 65536);
	
	// Print debug info.
	Serial1.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial1.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial1.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial1.printf("System Chip ID: %x\r\n", system_get_chip_id());
    Serial1.printf("SPI Flash ID: %x\r\n", spi_flash_get_id());
	
	mqtt = new MqttClient(MQTT_HOST, MQTT_PORT, onMqttReceived);
	
	Serial1.printf("\r\nCurrently running rom %d.\r\n", slot);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run success callback or failure callback depending on connection result.
	// Use a 20 second timeout.
	WifiEvents.onStationGotIP(OtaCore::connectOk);
	WifiEvents.onStationDisconnect(OtaCore::connectFail);
	
	// Call init callback.
	(*cb)();
}


// --- OTA UPDATE CALLBACK ---
void OtaCore::otaUpdate_CallBack(rBootHttpUpdate& update, bool result) {
	Serial1.println("In OTA callback...");
	//OtaCore::log(LOG_INFO, "In OTA callback...");
	if (result == true) { // success
		uint8 slot = rboot_get_current_rom();
		if (slot == 0) { slot = 1; } else { slot = 0; }
		
		// Set to boot new ROM and restart.
		Serial1.printf("Firmware updated, rebooting to ROM slot %d...\r\n", slot);
		OtaCore::log(LOG_INFO, "Firmware updated, restarting...");
		rboot_set_current_rom(slot);
		System.restart();
	} 
	else { // fail
		Serial1.println("Firmware update failed!");
		OtaCore::log(LOG_ERROR, "Firmware update failed.");
	}
}


// --- REGISTER TOPIC ---
// Registers an MQTT topic and an associated callback.
bool OtaCore::registerTopic(String topic, topicCallback cb) {
	OtaCore::mqtt->subscribe(topic);
	(*topicCallbacks)[topic] = cb;
	return true;
}


// --- DEREGISTER TOPIC ---
// Deregisters an MQTT topic.
bool OtaCore::deregisterTopic(String topic) {
	OtaCore::mqtt->unsubscribe(topic);
	if (topicCallbacks->contains(topic)) {
		topicCallbacks->remove(topic);
	}
	
	return true;
}


// --- PUBLISH ---
// Publishes the provided message to the MQTT broker on the specified topic.
bool OtaCore::publish(String topic, String message, int qos /* = 1 */) {
	OtaCore::mqtt->publishWithQoS(topic, message, qos);
	return true;
}


// --- OTA UPDATE ---
void OtaCore::otaUpdate() {
	Serial1.printf("Updating firmware from URL: %s...", OTA_URL);
	OtaCore::log(LOG_INFO, "Updating firmware from URL: " + String(OTA_URL));
	
	// Needs a clean object, otherwise if run before & failed, 
	// it'll not run again.
	if (otaUpdater) { delete otaUpdater; }
	otaUpdater = new rBootHttpUpdate();
	
	// Select ROM slot to flash. This is always the other slot than which we
	// last booted from.
	rboot_config bootconf = rboot_get_config();
	uint8 slot = bootconf.current_rom;
	if (slot == 0) { slot = 1; } else { slot = 0; }

	// Flash new ROM to other ROM slot.
	// HTTP URL is hardcoded, providing our WiFi MAC as UID parameter.
	otaUpdater->addItem(bootconf.roms[slot], OTA_URL + MAC);

	// Set a callback for the OTA procedure.
	otaUpdater->setCallback(OtaCore::otaUpdate_CallBack);
	otaUpdater->start(); // start update
}


// --- CHECK MQTT DISCONNECT ---
// Handle MQTT Disconnection events.
void OtaCore::checkMQTTDisconnect(TcpClient& client, bool flag) {	
	// Called whenever the MQTT connection has failed.
	if (flag == true) { Serial1.println("MQTT Broker disconnected."); }
	else { Serial1.println("MQTT Broker unreachable."); }
	
	// Restart connection attempt after 2 seconds.
	procTimer.initializeMs(2 * 1000, OtaCore::startMqttClient).start();
}


/* void onMessageDelivered(uint16_t msgId, int type) {
	Serial1.printf("Message with id %d and QoS %d was delivered successfully.", msgId, (type==MQTT_MSG_PUBREC? 2: 1));
} */


// --- START MQTT CLIENT ---
// Run MQTT client
void OtaCore::startMqttClient() {
	procTimer.stop();
	if (!mqtt->setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
	
#ifdef ENABLE_SSL
	//mqtt->connect(MAC, true, SSL_SERVER_VERIFY_LATER);
	mqtt->connect(MAC, MQTT_USERNAME, MQTT_PWD, true);
	mqtt->addSslOptions(SSL_SERVER_VERIFY_LATER);

	// DEBUG: list files on file system.
	/* Vector<String> filelist = fileList();    
    Serial1.println(" ");
    Serial1.println("Spiffs file system contents:");
    for (int i = 0; i < filelist.count(); i++ ) {
        Serial1.print(" ");
        Serial1.println(filelist.get(i) + " (" + String( fileGetSize(filelist.get(i))) + ")"     );
    } */
	
    Serial1.printf("Free Heap: %d\r\n", system_get_free_heap_size());
	
	// Read certificate and key in from SPIFFS.
	if (!fileExist("esp8266.client.crt.binary")) {
		Serial1.println("SSL CRT file is missing: esp8266.client.crt.binary.");
		return;
	}
	else if (!fileExist("esp8266.client.key.binary")) {
		Serial1.println("SSL key file is missing: esp8266.client.key.binary.");
		return;
	}
	
	unsigned int crtLength, keyLength;
	char* crtFile;
	char* keyFile;
	readIntoFileBuffer("esp8266.client.crt.binary", crtFile, crtLength);
	readIntoFileBuffer("esp8266.client.key.binary", keyFile, keyLength);
	
	Serial1.printf("keyLength: %d, crtLength: %d.\n", keyLength, crtLength);
    Serial1.printf("Free Heap: %d\r\n", system_get_free_heap_size());
	
	// If either file string is empty, opening the file failed.
	// TODO: handle.
	if (crtLength < 1 || keyLength < 1) {
		//
		Serial1.println("Failed to open certificate and/or key file.");
		return;
	}

	mqtt->setSslClientKeyCert((const uint8_t*) keyFile, keyLength,
							  (const uint8_t*) crtFile, crtLength, 0, true);
	delete[] keyFile;
	delete[] crtFile;
	
    Serial1.printf("Free Heap: %d\r\n", system_get_free_heap_size());
						
#else
	mqtt->connect(MAC, MQTT_USERNAME, MQTT_PWD, true);
#endif

	// Assign a disconnect callback function.
	mqtt->setCompleteDelegate(checkMQTTDisconnect);
	
	// Subscribe to relevant topics.
	mqtt->subscribe("upgrade");
	mqtt->subscribe("presence/tell");
	mqtt->subscribe("presence/ping");
	mqtt->subscribe("presence/restart/#");
	mqtt->subscribe("cc/" + MAC);
	
	delay(100); // Wait for 100 ms to let subscriptions to be processed.
	
	// Announce presence to C&C.
	mqtt->publish("cc/config", MAC);
	
	// TODO: set the module configuration using the stored 
	// configuration if available.
}


// --- CONNECT OK ---
// Will be called when the WiFi client has connected to the network.
void OtaCore::connectOk(IPAddress ip, IPAddress mask, IPAddress gateway) {
	Serial1.println("I'm CONNECTED. IP: " + ip.toString());
	
	// Get WiFi MAC address as this node's unique fingerprint.
	MAC = WifiStation.getMAC();
	Serial1.printf("MAC: %s.\n", MAC.c_str());
	
	// Set location from storage, if present.
	if (fileExist("location.txt")) {
		location = fileGetContent("location.txt");
	}
	else {
		location = MAC;
	}

	// Run MQTT client
	startMqttClient();
}


// --- CONNECT FAIL ---
// Will be called when WiFi station timeout was reached
void OtaCore::connectFail(String ssid, uint8_t ssidLength, uint8_t* bssid, uint8_t reason) {
	Serial1.println("I'm NOT CONNECTED. Need help :(");
	debugf("Disconnected from %s. Reason: %d", ssid.c_str(), reason);
	
	// Feed the watchdog timer to prevent a soft reset.
	WDT.alive();

	// Handle error.
	// Run success callback or failure callback depending on connection result.
	// Use a 20 second timeout.
	//WifiStation.waitConnection(OtaCore::connectOk, 20, OtaCore::connectFail);
	WifiEvents.onStationGotIP(OtaCore::connectOk);
	WifiEvents.onStationDisconnect(OtaCore::connectFail);
}


// --- ON MQTT RECEIVED ---
// Callback for MQTT message events.
void OtaCore::onMqttReceived(String topic, String message) {
	Serial1.print(topic);
	Serial1.print(":\n"); // Prettify alignment for printing
	Serial1.println(message);
	
	// If the topic is 'upgrade' with our MAC as message, start OTA update.
	if (topic == "upgrade" && message == MAC) {
		otaUpdate();
	}
	else if (topic == "presence/tell") {
		// Publish our MAC.
		mqtt->publish("presence/response", MAC);
	}
	else if (topic == "presence/ping") {
		// Send a ping response containing our MAC.
		mqtt->publish("presence/pong", MAC);
	}
	else if (topic == "presence/restart" && message == MAC) {
		System.restart();
	}
	else if (topic == "presence/restart/all") {
		System.restart();
	}
	else if (topic == "cc/" + MAC) {
		// We received a remote maintenance command. Execute it.
		// The message consists out of the following format:
		// <command string>;<payload string>
		int chAt = message.indexOf(';');
		String cmd = message.substring(0, chAt);
		++chAt;
		
		String msg = String((const char*) &(message[chAt]), (message.length() - chAt));
		
		Serial1.printf("Command: %s, Message: %s.\n", cmd.c_str(), msg.c_str());
		
		if (cmd == "mod") {
			// The second parameter should be the uint32 with bitflags.
			// These bit flags each match up with a module:
			// * 0x01: THPModule
			// * 0x02: CO2Module
			// * 0x04: JuraModule
			// * 0x08: JuraTermModule
			// * 0x10: MotionModule
			// * 0x20: PwmModule
			// * 0x40: IOModule
			// * 0x80: SwitchModule
			// ---
			// Of these, the CO2, Jura and JuraTerm modules are mutually
			// exclusive, since they all use the UART (Serial).
			// If two or more of these are still specified in the bitflags,
			// only the first module (DHTModule) will be enabled and the others
			// ignored.
			//
			// The Switch module currently uses the same pins as the i2c bus,
			// as well as a number of the PWM pins (D5, 6).
			// This means that it excludes all modules but the MotionModule and
			// those using the UART.
			//
			// TODO: maintain list of used/available pins in this module, 
			// handling the assigning of pins here as well (defaults and custom).
			// Idea: have modules obtain a pin via OtaCore's registry.
			if (msg.length() != 4) { // Must be 32-bit integer.
				Serial1.printf("Payload size wasn't 4 bytes: %d\n", msg.length());
				return; 
			}
			
			uint32 input = *((uint32*) &(msg[0]));
			uint32 new_mods = 0x0;
			
			Serial1.printf("Input: %x, Active: %x.\n", input, active_mods);
			
			// Compare the new configuration with the existing configuration.
			uint32 new_config = input ^ active_mods; // XOR comparison.
			if (new_config == 0x0) { 
				Serial1.print("New configuration was 0x0. No change.\n");
				log(LOG_INFO, "New configuration was 0x0. No change.");
				return; 
			}
			
			Serial1.printf("New configuration: %x.\n", new_config);
			log(LOG_INFO, "New configuration: " + new_config);
			
			// Set the new configuration.
			// Check whether the current module should be changed, then whether
			// the module is already active. If active, shut it down.
			if (new_config & MOD_TEMPERATURE_HUMIDITY) {
				if (active_mods & MOD_TEMPERATURE_HUMIDITY) {
					THPModule::shutdown();
					active_mods ^= MOD_TEMPERATURE_HUMIDITY;
				}
				else {
					THPModule::init();
					active_mods |= MOD_TEMPERATURE_HUMIDITY;
				}
			}
			
			if (new_config & MOD_MOTION) {
				if (active_mods & MOD_MOTION) {
					MotionModule::shutdown();
					active_mods ^= MOD_MOTION;
				}
				else {
					MotionModule::init();
					active_mods |= MOD_MOTION;
				}
			}
			
			if (new_config & MOD_PWM) {
				if (active_mods & MOD_PWM) {
					PwmModule::shutdown();
					active_mods ^= MOD_PWM;
				}
				else {
					PwmModule::init();
					active_mods |= MOD_PWM;
				}
			}
			
			if (new_config & MOD_IO) {
				if (active_mods & MOD_IO) {
					IOModule::shutdown();
					active_mods ^= MOD_IO;
				}
				else {
					IOModule::init();
					active_mods |= MOD_IO;
				}
			}
			
			if (new_config & MOD_SWITCH) {
				if (active_mods & MOD_SWITCH) {
					SwitchModule::shutdown();
					active_mods ^= MOD_SWITCH;
				}
				else {
					SwitchModule::init();
					active_mods |= MOD_SWITCH;
				}
			}
			
			// UART modules. Ensure mutual exclusivity. 
			// Keep a running tally of active UART modules. Refuse to turn on
			// new UART modules if one is already active.
			uint8 active = 0;
			if (active_mods & MOD_CO2) { active++; }
			if (active_mods & MOD_JURA) { active++; }
			if (active_mods & MOD_JURATERM) { active++; }
			
			Serial1.printf("Active UART modules: %u.\n", active);
			
			if (new_config & MOD_CO2 && active_mods & MOD_CO2) {
				CO2Module::shutdown();
				active_mods ^= MOD_CO2;
				new_config ^= MOD_CO2;
				active--;
			}
			
			if (new_config & MOD_JURA && active_mods & MOD_JURA) {
				JuraModule::shutdown();
				active_mods ^= MOD_JURA;
				new_config ^= MOD_JURATERM;
				active--;
			}
			
			if (new_config & MOD_JURATERM && active_mods & MOD_JURATERM) {
				JuraTermModule::shutdown();
				active_mods ^= MOD_JURATERM;
				new_config ^= MOD_JURATERM;
				active--;
			}

			// Enable the first active module, or abort if one is active already.
			if (active > 0) { return; }
			if (new_config & MOD_CO2) {
				CO2Module::init();
				active_mods |= MOD_CO2;
			}
			else if (new_config & MOD_JURA) {
				JuraModule::init();
				active_mods |= MOD_JURA;
			}
			else if (new_config & MOD_JURATERM) {
				JuraTermModule::init();
				active_mods |= MOD_JURATERM;
			}			
		}
		else if (cmd == "loc") {
			// Set the new location string if it's different.
			if (msg.length() < 1) { return; } // Incomplete message.
			if (location != msg) {
				location = msg;
				fileSetContent("location.txt", location); // Save to flash.
			}
		}
		else if (cmd == "dht" && msg.length() > 1) {
			// Send payload to the DHTModule's configuration method.
			THPModule::config(msg);
		}
		else if (cmd == "mod_active") {
			// Return currently active modules.
			if (active_mods == 0) {
				mqtt->publish("cc/response", MAC + ";0");
				return;
			}
			
			mqtt->publish("cc/response", MAC + ";" + String((const char*) &active_mods, 4));
		}
		else if (cmd == "version") {
			// Return firmware version.
			mqtt->publish("cc/response", MAC + ";" + version);
		}
	}
	else {
		if (topicCallbacks->contains(topic)) {
			(*((*topicCallbacks)[topic]))(message);
		}
	}
}


// --- LOG ---
void OtaCore::log(int level, String msg) {
	//if (level > logLevel) { return; }
	
	mqtt->publish("log/all", OtaCore::MAC + ";" + msg);
}


// --- START I2C ---
// Start the i2c communication on the specified pins.
bool OtaCore::starti2c() {
	// Start i2c comms.
	if (i2c_active) { return true; }
	
	// First perform a reset in case the slave device is stuck. For this we
	// pulse SCL 8 times.
	Wire.pins(sdaPin, sclPin);
	pinMode(sclPin, OUTPUT);
	for (int i = 0; i < 8; ++i) {
		digitalWrite(sclPin, HIGH);
		delayMicroseconds(3);
		digitalWrite(sclPin, LOW);
		delayMicroseconds(3);
	}
	  
	pinMode(sclPin, INPUT);
  
	// Next start the i2c bus.
	Wire.begin();
	i2c_active = true;
}
