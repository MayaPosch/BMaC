/*
	mth.h - Header file for the MQTT class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/10/18, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef LISTENER_H
#define LISTENER_H

#include <mosquittopp.h>

#include <string>
#include <map>

using namespace std;

#include <Poco/Mutex.h>

using namespace Poco;


struct NodeInfo;
struct ValveInfo;
struct SwitchInfo;

#include "nodes.h"


class Listener : public mosqpp::mosquittopp {
	map<string, NodeInfo> nodes;
	map<string, ValveInfo> valves;
	map<string, SwitchInfo> switches;
	Mutex nodesLock;
	Mutex valvesLock;
	Mutex switchesLock;
	bool heating;
	Mutex heatingLock;
	
public:
	Listener(string clientId, string host, int port);
	~Listener();
	
	void on_connect(int rc);
	void on_message(const struct mosquitto_message* message);
	void on_subscribe(int mid, int qos_count, const int* granted_qos);
	bool checkNodes();
	bool checkSwitch();
};

#endif
