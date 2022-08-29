/*
	controller.cpp - Core service for the BMaC system.
	
	Revision 0
	
	Features:
				- Waits for new node announcements and configures them.
				- Automatic discovery and configure functionality (TODO).
				- HTTP server for OTA update requests.
				
	Notes:
				- First argument to main is the location of the configuration
					file in INI format.
				
	2022/07/29, Maya Posch
*/


#include "mqtt_listener.h"

#include "httprequestfactory.h"
#include "nodes.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <csignal>
#include <mutex>
#include <condition_variable>

#include "sarge.h"
#include "INIReader.h"

//#include <Poco/Util/IniFileConfiguration.h>
//#include <Poco/AutoPtr.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>

//using namespace Poco::Util;
using namespace Poco;
using namespace Poco::Net;


// Globals.
std::condition_variable sigcv;


// --- LOG HANDLER ---
void logHandler(int level, std::string text) {
	std::cout << level << " - " << text << std::endl;
}


void signal_handler(int signal) {
	sigcv.notify_one();
}


int main(int argc, char* argv[]) {
	// Read configuration.
	/* std::string configFile;
	if (argc > 1) { configFile = argv[1]; }
	else { configFile = "config.ini"; }
	
	AutoPtr<IniFileConfiguration> config(new IniFileConfiguration(configFile));
	std::string mqtt_host = config->getString("MQTT.host", "localhost");
	int mqtt_port = config->getInt("MQTT.port", 1883);
	std::string configTopics = config->getString("MQTT.topics");
	std::string defaultFirmware = config->getString("Firmware.default", "ota_unified.bin"); */
	
	// Parse the command line arguments.
	Sarge sarge;
	sarge.setArgument("h", "help", "Get this help message.", false);
	sarge.setArgument("c", "configuration", "Path to configuration file.", true);
	sarge.setArgument("v", "version", "Output the software version and exit.", false);
	sarge.setDescription("Building Management and Control server. Details: https://github.com/MayaPosch/BMaC/.");
	sarge.setUsage("nymphcast_server <options>");
	
	sarge.parseArguments(argc, argv);
	
	if (sarge.exists("help")) {
		sarge.printHelp();
		return 0;
	}
	
	if (sarge.exists("version")) {
		std::cout << "BMaC server version: " << __VERSION << std::endl;
		return 0;
	}
	
	std::string config_file;
	if (!sarge.getFlag("configuration", config_file)) {
		std::cerr << "No configuration file provided in command line arguments." << std::endl;
		return 1;
	}
	
	std::cout << "Starting MQTT BMaC Control server...\n";
	
	// Read in the configuration.
	INIReader config(config_file);
	if (config.ParseError() != 0) {
		std::cerr << "Unable to load configuration file: " << config_file << std::endl;
		return 1;
	}
	
	std::string mqtt_host = config.Get("MQTT", "host", "localhost");
	int mqtt_port = config.GetInteger("MQTT", "port", 1883);
	std::string configTopics = config.Get("MQTT", "topics", "");
	std::string defaultFirmware = config.Get("Firmware", "default", "ota_unified.bin");
	
	std::cout << "Initialised MQTT library.\n";
	Listener listener;
	listener.init("BMaC_Controller", mqtt_host, mqtt_port);
	
	// Initialise the Nodes class.
	std::string influx_host = config.Get("", "Influx.host", "localhost");
	int influx_port = config.GetInteger("", "Influx.port", 8086);
	std::string influx_sec = config.Get("", "Influx.secure", "false");
	std::string influx_db = config.Get("", "Influx.db", "test");
	Nodes::init(defaultFirmware, influx_host, influx_port, influx_db, influx_sec, &listener);
	
	// Connect to the MQTT broker.
	if (!listener.connectBroker()) {
		std::cerr << "Failed to connect to broker." << std::endl;
		return 1;
	}
	
	// Subscribe to topics.
	std::vector<std::string> topics;
	topics.push_back("cc/config");		// C&C start.
	topics.push_back("cc/ui/config");
	topics.push_back("cc/nodes/new");
	topics.push_back("cc/nodes/update");
	topics.push_back("nsa/events/co2");
	topics.push_back("cc/firmware");
	topics.push_back("pwm/response");	// ACControl start.
	topics.push_back("io/response/#");
	topics.push_back("switch/response/#");
	
	std::map<std::string, std::string> series;
	StringTokenizer st(configTopics, ",", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
	for (StringTokenizer::Iterator it = st.begin(); it != st.end(); ++it) {
		std::string topic = std::string(*it);
		topics.push_back(topic);
		
		// Add name of the series to the 'series' map.
		StringTokenizer st1(topic, "/", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
		std::string s = st1[st1.count() - 1]; // Get last item.
		series.insert(std::pair<std::string, std::string>(topic, s));
	}
	
	for (uint32_t i = 0; i < topics.size(); ++i) {
		std::cout << "Subscribing to: " << topics[i] << "\n";
		if (!listener.addSubscription(topics[i])) {
			std::cerr << "Adding subscription failed. Aborting startup." << std::endl;
			return 1;
		}
	}
	
	// Initialise the HTTP server.
	uint16_t port = config.GetInteger("HTTP", "port", 8080);
	HTTPServerParams* params = new HTTPServerParams;
	params->setMaxQueued(100);
	params->setMaxThreads(10);
	HTTPServer httpd(new RequestHandlerFactory, port, params);
	httpd.start();
	
	// Publish the OTA URL as a retained message.
	// Use IP interface that is connected to the MQTT broker as target.
	std::string localIP = listener.getLocalIP();
	std::string m = "http://" + localIP + ":" + std::to_string(port) + "/ota.php?uid=";
	listener.publishMessage("cc/ota_url", m, 0, true);
	
	std::cout << "Created listener, entering loop...\n";
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Lock and wait.
	std::mutex sigmutex;
	std::unique_lock<std::mutex> lk(sigmutex);
	sigcv.wait(lk);
	
	/* int rc;
	while(1) {
		rc = listener.loop();
		if (rc){
			cout << "Disconnected. Trying to reconnect...\n";
			listener.reconnect();
		}
	} */
	
	std::cout << "Cleanup...\n";
	
	if (!listener.disconnectBroker()) {
		std::cerr << "Failed to disconnect from broker: " << std::endl;
		return 1;
	}

	return 0;
}
