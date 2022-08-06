/*
	nodes.h - Header file for the Nodes class.
	
	Revision 0
	
	Notes:
			- 
			
	2022/08/05, Maya Posch
*/


#ifndef NODES_H
#define NODES_H


#include <string>
#include <vector>

#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>

#include <Poco/Timer.h>

using namespace Poco;
using namespace Poco::Net;

class Listener;


struct NodeInfo {
	std::string uid;
	float posx;
	float posy;
	float current;		// current temperature for this node.
	float target;		// target temperature.
	bool ch0_state;		// state of this channel.
	uint8_t ch0_duty;		// current duty for this channel.
	bool ch0_valid;		// validation level.
	bool ch1_state;
	uint8_t ch1_duty;
	bool ch1_valid;
	bool ch2_state;
	uint8_t ch2_duty;
	bool ch2_valid;
	bool ch3_state;
	uint8_t ch3_duty;
	bool ch3_valid;
	uint8_t validate;	// current level of validation (by the MQTT class).
};


struct ValveInfo {
	std::string uid;
	uint8_t ch0_valve;
	uint8_t ch1_valve;
	uint8_t ch2_valve;
	uint8_t ch3_valve;
};


struct SwitchInfo {
	std::string uid;
	bool state; // false: cooling, true: heating.
};


#include "mqtt_listener.h"


class Nodes {
	static Data::Session* session;
	static bool initialized;
	static HTTPClientSession* influxClient;
	static std::string influxDb;
	static bool secure;
	static Listener* listener;
	static Timer* tempTimer;
	static Timer* nodesTimer;
	static Timer* switchTimer;
	static Nodes* selfRef;
	//static vector<string> uids;
	
public:
	static void init(std::string influxHost, int influxPort, std::string influxDb, 
										std::string influx_sec, Listener* listener);
	static void stop();
	static bool getNodeInfo(std::string uid, NodeInfo &info);
	static bool getValveInfo(std::string uid, ValveInfo &info);
	static bool getSwitchInfo(std::string uid, SwitchInfo &info);
	//static bool getNodesInfo(vector<NodeInfo> &info);
	static bool setTargetTemperature(std::string uid, float temp);
	static bool setCurrentTemperature(std::string uid, float temp);
	static bool setDuty(std::string uid, uint8_t ch0, uint8_t ch1, uint8_t ch2, uint8_t ch3);
	static bool setValves(std::string uid, bool ch0, bool ch1, bool ch2, bool ch3);
	static bool setSwitch(std::string uid, bool state);
	void updateCurrentTemperatures(Timer& timer);
	void checkNodes(Timer& timer);
	void checkSwitch(Timer& timer);
	static bool getUIDs(std::vector<std::string> &uids);
	static bool getSwitchUIDs(std::vector<std::string> &uids);
};

#endif
