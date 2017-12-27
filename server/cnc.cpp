/*
	cnc.cpp - Command & Control service for the BMaC system.
	
	Revision 1
	
	Features:
				- Waits for new node announcements and configures them.
				- Automatic discovery and configure functionality (TODO).
				- HTTP server for OTA update requests.
				
	Notes:
				- First argument to main is the location of the configuration
					file in INI format.
				
	2017/03/21, Maya Posch <posch@synyx.de>
	2017/12/21, Maya Posch
*/


#include "listener.h"

#include <iostream>
#include <string>

using namespace std;

#include <Poco/Util/IniFileConfiguration.h>
#include <Poco/AutoPtr.h>
#include <Poco/Net/HTTPServer.h>

using namespace Poco::Util;
using namespace Poco;
using namespace Poco::Net;

#include "httprequestfactory.h"


int main(int argc, char* argv[]) {
	cout << "Starting MQTT BMaC Command & Control server...\n";
	
	int rc;
	mosqpp::lib_init();
	
	cout << "Initialised C++ Mosquitto library.\n";
	
	// Read configuration.
	string configFile;
	if (argc > 1) { configFile = argv[1]; }
	else { configFile = "config.ini"; }
	
	// Initialise the MQTT client.
	AutoPtr<IniFileConfiguration> config(new IniFileConfiguration(configFile));
	string mqtt_host = config->getString("MQTT.host", "localhost");
	int mqtt_port = config->getInt("MQTT.port", 1883);
	string defaultFirmware = config->getString("Firmware.default", "ota_unified.bin");
	
	Listener listener("Command_and_Control", mqtt_host, mqtt_port, defaultFirmware);
	
	// Initialise the HTTP server.
	UInt16 port = config->getInt("HTTP.port", 8080);
	HTTPServerParams* params = new HTTPServerParams;
	params->setMaxQueued(100);
	params->setMaxThreads(10);
	HTTPServer httpd(new RequestHandlerFactory, port, params);
	httpd.start();
	
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
