/*
	cnc.cpp - Command & Control service for the BMaC system.
	
	Revision 0
	
	Features:
				- Waits for new node announcements and configures them.
				- Automatic discovery and configure functionality (TODO).
				
	Notes:
				- First argument to main is the location of the configuration
					file in INI format.
				
	2017/03/21, Maya Posch <posch@synyx.de>
*/


#include "listener.h"

#include <iostream>
#include <string>

using namespace std;

#include <Poco/Util/IniFileConfiguration.h>
#include <Poco/AutoPtr.h>

using namespace Poco::Util;
using namespace Poco;


int main(int argc, char* argv[]) {
	cout << "Starting MQTT BMaC Command & Control server...\n";
	
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
	
	Listener listener("Command_and_Control", mqtt_host, mqtt_port);
	
	cout << "Created listener, entering loop...\n";
	
	while(1) {
		rc = listener.loop();
		if (rc){
			cout << "Disconnected. Trying to reconnect...\n";
			listener.reconnect();
		}
	}
	
	cout << "Cleanup...\n";

	mosqpp::lib_cleanup();

	return 0;
}
