/*
	nodes.h - Header file for the Nodes class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/10/29, Maya Posch <posch@synyx.de>
*/


#ifndef NODES_H
#define NODES_H


#include <string>
#include <vector>

using namespace std;

#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>

#include <Poco/Timer.h>

using namespace Poco;
using namespace Poco::Net;

class Listener;


struct NodeInfo {
	string uid;
	float posx;
	float posy;
	float current;		// current temperature for this node.
	float target;		// target temperature.
	bool ch0_state;		// state of this channel.
	UInt8 ch0_duty;		// current duty for this channel.
	bool ch0_valid;		// validation level.
	bool ch1_state;
	UInt8 ch1_duty;
	bool ch1_valid;
	bool ch2_state;
	UInt8 ch2_duty;
	bool ch2_valid;
	bool ch3_state;
	UInt8 ch3_duty;
	bool ch3_valid;
	UInt8 validate;	// current level of validation (by the MQTT class).
};


struct ValveInfo {
	string uid;
	UInt8 ch0_valve;
	UInt8 ch1_valve;
	UInt8 ch2_valve;
	UInt8 ch3_valve;
};


struct SwitchInfo {
	string uid;
	bool state; // false: cooling, true: heating.
};


#include "listener.h"


class Nodes {
	static Data::Session* session;
	static bool initialized;
	static HTTPClientSession* influxClient;
	static string influxDb;
	static bool secure;
	static Listener* listener;
	static Timer* tempTimer;
	static Timer* nodesTimer;
	static Timer* switchTimer;
	static Nodes* selfRef;
	//static vector<string> uids;
	
public:
	static void init(string influxHost, int influxPort, string influxDb, string influx_sec, Listener* listener);
	static void stop();
	static bool getNodeInfo(string uid, NodeInfo &info);
	static bool getValveInfo(string uid, ValveInfo &info);
	static bool getSwitchInfo(string uid, SwitchInfo &info);
	//static bool getNodesInfo(vector<NodeInfo> &info);
	static bool setTargetTemperature(string uid, float temp);
	static bool setCurrentTemperature(string uid, float temp);
	static bool setDuty(string uid, UInt8 ch0, UInt8 ch1, UInt8 ch2, UInt8 ch3);
	static bool setValves(string uid, bool ch0, bool ch1, bool ch2, bool ch3);
	static bool setSwitch(string uid, bool state);
	void updateCurrentTemperatures(Timer& timer);
	void checkNodes(Timer& timer);
	void checkSwitch(Timer& timer);
	static bool getUIDs(vector<string> &uids);
	static bool getSwitchUIDs(vector<string> &uids);
};

#endif
