/*
	influx_mqtt.cpp - Proxy to convert MQTT events into InfluxDB HTTP protocol.
	
	Revision 0
	
	Features:
				- 
				
	Notes:
				- First argument to main is the location of the configuration
					file in INI format.
				
	2017/02/09, Maya Posch <posch@synyx.de>
*/


#include "mth.h"

#include <iostream>
#include <string>

using namespace std;

#include <Poco/Util/IniFileConfiguration.h>
#include <Poco/AutoPtr.h>

using namespace Poco::Util;
using namespace Poco;


int main(int argc, char* argv[]) {
	cout << "Starting MQTT to InfluxDB-REST listener...\n";
	
	int rc;
	mosqpp::lib_init();
	
	cout << "Initialised C++ Mosquitto library.\n";
	
	// Read configuration.
	string configFile;
	if (argc > 1) { configFile = argv[1]; }
	else { configFile = "config.ini"; }
	
	AutoPtr<IniFileConfiguration> config(new IniFileConfiguration(configFile));
	string mqtt_host = config->getString("MQTT.host", "localhost");
	int mqtt_port = config->getInt("MQTT.port", 1883);
	string topics = config->getString("MQTT.topics");
	
	string influx_host = config->getString("Influx.host", "localhost");
	int influx_port = config->getInt("Influx.port", 8086);
	string influx_sec = config->getString("Influx.secure", "false");
	string influx_db = config->getString("Influx.db", "test");
	
	MtH mth("MQTT-to-InfluxDB", mqtt_host, mqtt_port, topics, influx_host, 
											influx_port, influx_db, influx_sec);
	
	cout << "Created listener, entering loop...\n";
	
	while(1) {
		rc = mth.loop();
		if (rc){
			cout << "Disconnected. Trying to reconnect...\n";
			mth.reconnect();
		}
	}
	
	cout << "Cleanup...\n";

	mosqpp::lib_cleanup();

	return 0;
}
