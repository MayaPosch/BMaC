/*
	nodes.cpp - Implementation of the Nodes class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/10/29, Maya Posch <posch@synyx.de>
*/


#include "nodes.h"

#include <iostream>

using namespace std;

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Net;
using namespace Poco::Data::Keywords;


// Static initialisations.
Data::Session* Nodes::session;
bool Nodes::initialized = false;
HTTPClientSession* Nodes::influxClient;
string Nodes::influxDb;
Listener* Nodes::listener;
bool Nodes::secure;
Timer* Nodes::tempTimer;
Timer* Nodes::nodesTimer;
Timer* Nodes::switchTimer;
Nodes* Nodes::selfRef;
//vector<string> Nodes::uids;


// --- INIT ---
// Initialise the static class.
void Nodes::init(string influxHost, int influxPort, string influxDb, 
									string influx_sec, Listener* listener) {
	// Assign parameters.
	Nodes::listener = listener;
	Nodes::influxDb = influxDb;
	if (influx_sec == "true") { 
		cout << "Connecting with HTTPS..." << std::endl;
		influxClient = new HTTPSClientSession(influxHost, influxPort);
		secure = true; 
	} 
	else {
		cout << "Connecting with HTTP..." << std::endl;
		influxClient = new HTTPClientSession(influxHost, influxPort);
		secure = false; 
	}
	
	// Set up SQLite database link.
	Data::SQLite::Connector::registerConnector();
	session = new Data::Session("SQLite", "nodes.db");
	
	// Ensure the database has a valid nodes table.
	// Layout:
	// * uid TEXT UNIQUE
	// * posx FLOAT
	// * posy FLOAT
	// * current FLOAT
	// * target FLOAT
	// * ch0_io INT
	// * ch0_state INT
	// * ch0_duty INT
	// * ch1_io INT
	// * ch1_state INT
	// * ch1_duty INT
	// * ch2_io INT
	// * ch2_state INT
	// * ch2_duty INT
	// * ch3_io INT
	// * ch3_state INT
	// * ch3_duty INT
	(*session) << "CREATE TABLE IF NOT EXISTS nodes (uid TEXT UNIQUE, \
		posx FLOAT, \
		posy FLOAT, \
		current FLOAT, \
		target FLOAT, \
		ch0_io INT, \
		ch0_state INT, \
		ch0_duty INT, \
		ch1_io INT, \
		ch1_state INT, \
		ch1_duty INT, \
		ch2_io INT, \
		ch2_state INT, \
		ch2_duty INT, \
		ch3_io INT, \
		ch3_state INT, \
		ch3_duty INT)", now;
		
	// Ensure we have a valid valves table.
	// * uid TEXT UNIQUE
	// * ch0_valve INT
	// * ch1_valve INT
	// * ch2_valve INT
	// * ch3_valve INT
	(*session) << "CREATE TABLE IF NOT EXISTS valves (uid TEXT UNIQUE, \
		ch0_valve INT, \
		ch1_valve INT, \
		ch2_valve INT, \
		ch3_valve INT)", now;
		
	// Ensure we have a valid switch table.
	// * uid TEXT UNIQUE
	// * state INT
	(*session) << "CREATE TABLE IF NOT EXISTS switches (uid TEXT UNIQUE, \
		state INT)", now;
		
	// Start the timers for checking the condition of each node.
	// One for the current PWM status (active pins, duty cycle), one for 
	// updating the current temperature measured by each node.
	selfRef = new Nodes;
	tempTimer = new Timer(1000, 30 * 1000); 		// wait 1 s, 30 s interval.
	nodesTimer = new Timer(5000, 30 * 1000); 		// wait 5 s, 30 s interval.
	switchTimer = new Timer(1000, 60 * 60 * 1000);	// wait 1 s, 1 hour interval.
	TimerCallback<Nodes> tempCb(*selfRef, &Nodes::updateCurrentTemperatures);
	TimerCallback<Nodes> nodesCb(*selfRef, &Nodes::checkNodes);
	TimerCallback<Nodes> switchCb(*selfRef, &Nodes::checkSwitch);
	tempTimer->start(tempCb);
	nodesTimer->start(nodesCb);
	switchTimer->start(switchCb);
	
	// Done.
	initialized = true;
}


// --- STOP ---
// Stop this class.
void Nodes::stop() {
	tempTimer->stop();
	nodesTimer->stop();
	delete tempTimer;
	delete nodesTimer;
	delete influxClient;
	delete session;
	delete selfRef;
}


// --- GET NODE INFO ---
// Fills the provided struct with information on the specified node.
bool Nodes::getNodeInfo(string uid, NodeInfo &info) {
	if (!initialized) { return false; }
	
	cout << "Getting node info for UID: " << uid << endl;
	
	Data::Statement select(*session);
	info.uid = uid;
	select << "SELECT posx, posy, current, target, ch0_state, ch0_duty, \
				ch1_state, ch1_duty, ch2_state, ch2_duty, ch3_state, ch3_duty \
				FROM nodes WHERE uid=?",
				into (info.posx),
				into (info.posy),
				into (info.current),
				into (info.target),
				into (info.ch0_state),
				into (info.ch0_duty),
				into (info.ch1_state),
				into (info.ch1_duty),
				into (info.ch2_state),
				into (info.ch2_duty),
				into (info.ch3_state),
				into (info.ch3_duty),
				use (uid);
				
	size_t rows = select.execute();
	if (rows != 1) { return false; }
	
	return true;
}


// --- GET VALVE INFO ---
//
bool Nodes::getValveInfo(string uid, ValveInfo &info) {
	if (!initialized) { return false; }
	
	cout << "Getting valve info for UID: " << uid << endl;
	
	Data::Statement select(*session);
	info.uid = uid;
	select << "SELECT ch0_valve, ch1_valve, ch2_valve, ch3_valve FROM valves WHERE uid=?",
				into (info.ch0_valve),
				into (info.ch1_valve),
				into (info.ch2_valve),
				into (info.ch3_valve),
				use (uid);
				
	size_t rows = select.execute();
	if (rows != 1) { return false; }
	
	return true;
}


// --- GET SWITCH INFO ---
bool Nodes::getSwitchInfo(string uid, SwitchInfo &info) {
	if (!initialized) { return false; }
	
	cout << "Getting switch info for UID: " << uid << endl;
	
	Data::Statement select(*session);
	info.uid = uid;
	select << "SELECT state FROM switches WHERE uid=?",
				into (info.state),
				use (uid);
				
	size_t rows = select.execute();
	if (rows != 1) { return false; }
	
	return true;
}


// --- GET NODES INFO ---
// Fills the provided vector with information on all active nodes.
/* bool Nodes::getNodesInfo(vector<NodeInfo> &info) {
	if (!initialized) { return false; }
	Data::Statement select(*session);
	info.clear();
	select << "SELECT posx, posy, current, target, ch0_state, ch0_duty, \
				ch1_state, ch1_duty, ch2_state, ch2_duty, ch3_state, ch3_duty \
				FROM nodes",
				into (info.posx),
				into (info.posy),
				into (info.current),
				into (info.target),
				into (info.ch0_state),
				into (info.ch0_duty),
				into (info.ch1_state),
				into (info.ch1_duty),
				into (info.ch2_state),
				into (info.ch2_duty),
				into (info.ch3_state),
				into (info.ch3_duty);
				
	size_t rows = select.execute();
	if (rows != 1) { return false; }
	
	return true;
} */


// --- SET TARGET TEMPERATURE ---
// Set a new target temperature for the specified node.
bool Nodes::setTargetTemperature(string uid, float temp) {
	if (!initialized) { return false; }
	
	cout << "Setting target temperature for UID: " << uid << endl;
	
	Data::Statement update(*session);
	update << "UPDATE nodes SET target = ? WHERE uid = ?",
			use(temp),
			use(uid),
			now;
			
	return true;
}


// --- SET CURRENT TEMPERATURE ---
// Set a new current temperature for the specified node.
bool Nodes::setCurrentTemperature(string uid, float temp) {
	if (!initialized) { return false; }
	
	cout << "Updating current temperature for node " << uid << " to " 
			<< temp << endl;
	
	Data::Statement update(*session);
	update << "UPDATE nodes SET current = ? WHERE uid = ?",
			use(temp),
			use(uid),
			now;
			
	return true;
}


// --- SET DUTY ---
// Update the duty for the specified node.
bool Nodes::setDuty(string uid, UInt8 ch0, UInt8 ch1, UInt8 ch2, UInt8 ch3) {
	if (!initialized) { return false; }
	
	cout << "Setting duty for UID: " << uid << endl;
	
	Data::Statement update(*session);
	update << "UPDATE nodes SET ch0_duty = ?, ch1_duty = ?, ch2_duty = ?, ch3_duty = ? WHERE uid = ?",
			use(ch0),
			use(ch1),
			use(ch2),
			use(ch3),
			use(uid),
			now;
			
	return true;
}


// --- SET VALVES ---
// Change the configuration of the valves for the specified node.
bool Nodes::setValves(string uid, bool ch0, bool ch1, bool ch2, bool ch3) {
	if (!initialized) { return false; }
	
	cout << "Setting valve state for UID: " << uid << endl;
	
	Data::Statement update(*session);
	update << "UPDATE valves SET ch0_valve = ?, ch1_valve = ?, ch2_valve = ?, ch3_valve = ? WHERE uid = ?",
			use(ch0),
			use(ch1),
			use(ch2),
			use(ch3),
			use(uid),
			now;
			
	return true;
}


// --- SET SWITCH ---
// Change the state of the switch with the provided UID.
bool Nodes::setSwitch(string uid, bool state) {
	if (!initialized) { return false; }
	
	cout << "Setting switch state for UID: " << uid << " to " << state << endl;
	
	Data::Statement update(*session);
	update << "UPDATE switches SET state = ? WHERE uid = ?",
			use(state),
			use(uid),
			now;
			
	return true;
}


// --- UPDATE CURRENT TEMPERATURES ---
// Request the current temperatures from the Influx database using the MACs.
void Nodes::updateCurrentTemperatures(Timer& /*timer*/) {
	if (!initialized) { return; }
	
	cout << "Updating current temperatures...\n";
	
	// Get the UIDs.
	std::vector<string> uids;
	if (!getUIDs(uids)) { 
		cerr << "UpdateCurrentTemperatures: Failed to get the UIDs.\n";
		return; 
	}
	
	cout << "Contacting Influx database...\n";
	
	int uidsl = uids.size();
	string responseStr;
	for (unsigned int i = 0; i < uidsl; ++i) {
		// Send message.
		string query = "SELECT%20\"value\"%20FROM%20\"temperature\"%20WHERE%20\"location\"='"
							+ uids[i] + "'%20ORDER%20BY%20time%20DESC%20LIMIT%201";
		try {			
			HTTPRequest request(HTTPRequest::HTTP_GET, "/query?db=" + influxDb + 
										"&q=" + query, HTTPMessage::HTTP_1_1);
										
			//cout << "Request: " << request.getURI() << endl;
												
			influxClient->sendRequest(request);
			
			HTTPResponse response;
			istream &rs = influxClient->receiveResponse(response);
			HTTPResponse::HTTPStatus status = response.getStatus();
			if (status != HTTPResponse::HTTP_OK/*  && 
					status != HTTPResponse::HTTP_NO_CONTENT */) {
				cerr << "Received InfluxDB error: " << response.getReason() << "\n";
				cerr << "Aborting temperature update loop.\n";
				break;
			}
			
			StreamCopier::copyToString(rs, responseStr);
		}
		catch (Exception& exc) {
			cout << "Exception caught while attempting to connect." << endl;
			cerr << exc.displayText() << endl;
			return;
		}
		
		//cout << "Influx response: " << responseStr << endl;
		
		// Process response.
		// Write the obtained temperature for each node to the database.
		Parser parser;
		Dynamic::Var result = parser.parse(responseStr);
		Object::Ptr object = result.extract<Object::Ptr>();
		if (object->has("error")) { // Error occurred.
			string err = object->getValue<string>("error");
			cerr << "Error from Influx DB for current temperature: " << err << endl;
			continue;
		}
		
		if (!object->isArray("results")) {
			cerr << "Results content wasn't an array. Skipping.\n";
			continue;
		}
		
		Array::Ptr results = object->getArray("results");
		Dynamic::Array da = *results;
		float newTemp = da[0]["series"][0]["values"][0][1];
		setCurrentTemperature(uids[i], newTemp);
	}
}


// --- CHECK NODES ---
// Triggers the 'checkNodes' function in the MQTT listener.
void Nodes::checkNodes(Timer& /*timer*/) {
	if (!initialized) { return; }
	
	cout << "Calling Listener::checkNodes().\n";
	
	listener->checkNodes();
}


// --- CHECK SWITCH ---
// Check the current status of the heating/cooling switch.
void Nodes::checkSwitch(Timer& timer) {
	if (!initialized) { return; }
	
	listener->checkSwitch();
}


// --- GET UIDS ---
// Get all UIDs of currently active nodes.
bool Nodes::getUIDs(std::vector<string> &uids) {
	if (!initialized) { return false; }
	
	cout << "Getting UIDs...\n";
	
	uids.clear();
	
	//std::vector<string> temp;
	
	Data::Statement select(*session);
	string uid;
	select << "SELECT uid FROM nodes", into (uid), range(0, 1);
	//select << "SELECT uid FROM nodes", into (temp);
	
	while (!select.done()) {
		select.execute();
		cout << "Found UID: " << uid << endl;
		uids.push_back(uid);
	}
	
	//size_t rows = select.execute();
	//cout << "Rows: " << rows << endl;
	
	cout << "Found " << uids.size() << " nodes.\n";
	
	//uids = temp;
	
	return true;
}


// --- GET SWITCH UIDS ---
bool Nodes::getSwitchUIDs(std::vector<string> &uids) {
	if (!initialized) { return false; }
	
	cout << "Getting switch UIDs...\n";
	
	uids.clear();
	
	Data::Statement select(*session);
	string uid;
	select << "SELECT uid FROM switches", into (uid), range(0, 1);
	
	while (!select.done()) {
		select.execute();
		cout << "Found switch UID: " << uid << endl;
		uids.push_back(uid);
	}
	
	cout << "Found " << uids.size() << " nodes.\n";
	
	return true;
}
