/*
	mqtt_listener.h - Header for MQTT listener of the BMaC controller.
	
	Revision 0.
	
	2022/07/29, Maya Posch
*/

#pragma once
#ifndef MQTT_LISTENER_H
#define MQTT_LISTENER_H

#include <nymphmqtt/client.h>

#include <string>
#include <map>

#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Mutex.h>
#include <Poco/Net/HTTPSClientSession.h>

using namespace Poco;

struct NodeInfo;
struct ValveInfo;
struct SwitchInfo;


class Listener {
	NmqttClient client;
	NmqttBrokerConnection conn;
	int handle;
	std::string host;
	int port;
	Data::Session* session;
	std::string defaultFirmware;
	
	Poco::Net::HTTPClientSession* influxClient;
	std::string influxDb;
	bool secure;
	
	std::map<std::string, std::string> series;
	//std::map<std::string, NodeInfo> nodes;
	std::map<std::string, ValveInfo> valves;
	std::map<std::string, SwitchInfo> switches;
	//Mutex nodesLock;
	Mutex valvesLock;
	Mutex switchesLock;
	bool heating;
	Mutex heatingLock;
	
	void logHandler(int level, std::string text);
	void messageHandler(int handle, std::string topic, std::string payload);
	
public:
	Listener();
	~Listener();
	
	bool init(std::string clientId = "BMaC-controller", std::string host = "localhost", int port = 1883);
	bool connectBroker();
    bool disconnectBroker();
	bool addSubscription(std::string topic);
	bool publishMessage(std::string topic, std::string msg, uint8_t qos = 0, bool retain = false);
	bool checkNodes();
	bool checkSwitch();
	
	std::string getLocalIP();
};

#endif
