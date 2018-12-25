/*
	accontrol.cpp - AirConditioning Control service for the BMaC system.
	
	Revision 0
	
	Features:
				- Actively polls configured nodes and keeps them updated.
				- REST API 
				
	Notes:
				- First argument to main is the location of the configuration
					file in INI format.
				
	2017/10/18, Maya Posch <posch@synyx.de>
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
#include "nodes.h"


int main(int argc, char* argv[]) {
	cout << "Starting MQTT BMaC AirConditioning Control server...\n";
	
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
	
	// Start the MQTT listener.
	Listener listener("ACControl", mqtt_host, mqtt_port);
	
	cout << "Created listener, entering loop...\n";
	
	// Initialise the Nodes class.
	string influx_host = config->getString("Influx.host", "localhost");
	int influx_port = config->getInt("Influx.port", 8086);
	string influx_sec = config->getString("Influx.secure", "false");
	string influx_db = config->getString("Influx.db", "test");
	Nodes::init(influx_host, influx_port, influx_db, influx_sec, &listener);
	
	// Initialise the HTTP server.
	UInt16 port = config->getInt("HTTP.port", 8081);
	HTTPServerParams* params = new HTTPServerParams;
	params->setMaxQueued(100);
	params->setMaxThreads(10);
	HTTPServer httpd(new RequestHandlerFactory, port, params);
	httpd.start();
	
	while(1) {
		rc = listener.loop();
		if (rc){
			cout << "Disconnected. Trying to reconnect...\n";
			listener.reconnect();
		}
	}
	
	cout << "Cleanup...\n";

	mosqpp::lib_cleanup();
	httpd.stop();
	Coffeenet::stop();

	return 0;
}
