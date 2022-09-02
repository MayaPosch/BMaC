/*
	nodes.cpp - Implementation of the Nodes class.
	
	Revision 0
	
	Notes:
			- 
			
	2022/08/05, Maya Posch
*/


#include "nodes.h"

#include <iostream>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Data/SQLite/SQLiteException.h>

using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Net;
using namespace Poco::Data::Keywords;


// Static initialisations.
Data::Session* Nodes::session;
bool Nodes::initialized = false;
HTTPClientSession* Nodes::influxClient;
std::string Nodes::influxDb;
Listener* Nodes::listener;
bool Nodes::secure;
std::string Nodes::defaultFirmware;
std::vector<NodeInfo> Nodes::nodes;
std::vector<NodeInfo> Nodes::newNodes;
Timer* Nodes::tempTimer;
Timer* Nodes::nodesTimer;
Timer* Nodes::switchTimer;
Nodes* Nodes::selfRef;
//vector<string> Nodes::uids;


// --- INIT ---
// Initialise the static class.
void Nodes::init(std::string defaultFirmware, std::string influxHost, int influxPort, std::string influxDb, 
									std::string influx_sec, Listener* listener) {
	// Assign parameters.
	Nodes::defaultFirmware = defaultFirmware;
	Nodes::listener = listener;
	Nodes::influxDb = influxDb;
	if (influx_sec == "true") { 
		std::cout << "Connecting with HTTPS..." << std::endl;
		influxClient = new HTTPSClientSession(influxHost, influxPort);
		secure = true; 
	} 
	else {
		std::cout << "Connecting with HTTP..." << std::endl;
		influxClient = new HTTPClientSession(influxHost, influxPort);
		secure = false; 
	}
	
	// Set up SQLite database link.
	Data::SQLite::Connector::registerConnector();
	session = new Data::Session("SQLite", "nodes.db");
	
	// Ensure the database has a valid nodes table.
	// Layout:
	// * uid TEXT UNIQUE
	// * location TEXT
	// * modules INT
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
		location TEXT, \
		modules INT, \
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
		
	std::cout << "Checked for 'nodes' table." << std::endl;
		
	// Ensure we have a valid firmware table.
	(*session) << "CREATE TABLE IF NOT EXISTS firmware (uid TEXT UNIQUE, \
		file TEXT)", now;
		
	std::cout << "Checked for 'firmware' table." << std::endl;
		
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
		
	std::cout << "Checked for 'valves' table." << std::endl;
		
	// Ensure we have a valid switch table.
	// * uid TEXT UNIQUE
	// * state INT
	(*session) << "CREATE TABLE IF NOT EXISTS switches (uid TEXT UNIQUE, \
		state INT)", now;
		
	std::cout << "Checked for 'switches' table." << std::endl;
	std::cout << "Reading in nodes from 'nodes' table..." << std::endl;
		
	// Load the node information from the database into memory.
	try {
		Data::Statement select(*session);
		NodeInfo info;
		select << "SELECT uid, location, modules, posx, posy, current, target, ch0_state, ch0_duty, \
					ch1_state, ch1_duty, ch2_state, ch2_duty, ch3_state, ch3_duty \
					FROM nodes",
					into (info.uid),
					into (info.location),
					into (info.modules),
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
					range(0, 1);
					
		while (!select.done()) {
			select.execute();
			if (info.uid.length() > 5) {
				std::cout << "Node: " << info.uid << std::endl;
				nodes.push_back(info);
			}
		}
	}
	catch (Poco::Data::SQLite::InvalidSQLStatementException &e) {
		//
		std::cerr << "Exception: " << e.message() << std::endl;
		return;
	}
	
	std::cout << "Read " << nodes.size() << " nodes from the database." << std::endl;
		
	// Start the timers for checking the condition of each node.
	// One for the current PWM status (active pins, duty cycle), one for 
	// updating the current temperature measured by each node.
	/* selfRef = new Nodes;
	tempTimer = new Timer(1000, 30 * 1000); 		// wait 1 s, 30 s interval.
	nodesTimer = new Timer(5000, 30 * 1000); 		// wait 5 s, 30 s interval.
	switchTimer = new Timer(1000, 60 * 60 * 1000);	// wait 1 s, 1 hour interval.
	TimerCallback<Nodes> tempCb(*selfRef, &Nodes::updateCurrentTemperatures);
	TimerCallback<Nodes> nodesCb(*selfRef, &Nodes::checkNodes);
	TimerCallback<Nodes> switchCb(*selfRef, &Nodes::checkSwitch);
	tempTimer->start(tempCb);
	nodesTimer->start(nodesCb);
	switchTimer->start(switchCb); */
	
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
bool Nodes::getNodeInfo(std::string uid, NodeInfo &info) {
	if (!initialized) { return false; }
	
	std::cout << "Getting node info for UID: " << uid << std::endl;
	
	Data::Statement select(*session);
	info.uid = uid;
	select << "SELECT location, modules, posx, posy, current, target, ch0_state, ch0_duty, \
				ch1_state, ch1_duty, ch2_state, ch2_duty, ch3_state, ch3_duty \
				FROM nodes WHERE uid=?",
				into (info.location),
				into (info.modules),
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
	if (rows != 1) { 
		// Add unknown UIDs to an in-memory list, for retrieval by management software.
		std::cout << "Adding new node with UID " << uid << " to unassigned list." << std::endl;
		info.uid = uid;
		newNodes.push_back(info);
		
		return false; 
	}
	
	return true;
}


// --- UPDATE NODE INFO ---
bool Nodes::updateNodeInfo(std::string uid, NodeInfo &node) {
	std::cout << "Updating nodes table..." << std::endl;
	
	// Update a node if it already exists, otherwise insert it as a new entry.
	Data::Statement insert(*session);
		insert << "INSERT OR REPLACE INTO nodes (uid, location, modules, posx, posy) \
					VALUES(?, ?, ?, ?, ?)",
				use(node.uid),
				use(node.location),
				use(node.modules),
				use(node.posx),
				use(node.posy),
				now;
				
	std::cout << "Updating firmware table..." << std::endl;
				
	// If the UID doesn't exist yet in the firmware table, insert the default.
	(*session) << "INSERT OR ABORT INTO firmware VALUES(?, ?)",
			use(node.uid),
			use(defaultFirmware),
			now;
			
	std::cout << "Updating node via MQTT..." << std::endl;
				
	// Update target node.
	std::string topic = "cc/" + uid;
	std::string msg = "loc;" + node.location;
	listener->publishMessage(topic, msg);
	
	msg = "mod;";
	msg += std::string(((char*) &(node.modules)), 4);
	listener->publishMessage(topic, msg);
	
	// If newly assigned node, from from unassigned list, assign to assigned list.
	
	
	return true;
}


// --- DELETE NODE INFO ---
bool Nodes::deleteNodeInfo(std::string uid) {
	// Update a node if it already exists, otherwise insert it as a new entry.
	Data::Statement insert(*session);
		insert << "DELETE FROM nodes WHERE uid=?",
				use(uid),
				now;
		
	return true;
}


// --- GET VALVE INFO ---
//
bool Nodes::getValveInfo(std::string uid, ValveInfo &info) {
	if (!initialized) { return false; }
	
	std::cout << "Getting valve info for UID: " << uid << std::endl;
	
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
bool Nodes::getSwitchInfo(std::string uid, SwitchInfo &info) {
	if (!initialized) { return false; }
	
	std::cout << "Getting switch info for UID: " << uid << std::endl;
	
	Data::Statement select(*session);
	info.uid = uid;
	select << "SELECT state FROM switches WHERE uid=?",
				into (info.state),
				use (uid);
				
	size_t rows = select.execute();
	if (rows != 1) { return false; }
	
	return true;
}


// ---	NODES TO JSON ---
// Returns a JSON array containing the assigned nodes.
// TODO: Buffer the output.
std::string Nodes::nodesToJson() {
	std::string out = "[ ";
	
	std::cout << "Converting " << nodes.size() << " nodes to JSON..." << std::endl;
	
	for (int i = 0; i < nodes.size(); ++i) {
		out += "{ \"uid\": \"" + nodes[i].uid + "\", ";
		out += "\"location\": \"" + nodes[i].location + "\", ";
		
		// Modules section.
		// The bit flags match up with a module:
		// * 0x01: 	THPModule
		// * 0x02: 	CO2Module
		// * 0x04: 	JuraModule
		// * 0x08: 	JuraTermModule
		// * 0x10: 	MotionModule
		// * 0x20: 	PwmModule
		// * 0x40: 	IOModule
		// * 0x80: 	SwitchModule
		// * 0x100: PlantModule
		// ---
		// Of these, the CO2, Jura and JuraTerm modules are mutually
		// exclusive, since they all use the UART (Serial).
		// If two or more of these are still specified in the bitflags,
		// only the first module will be enabled and the others
		// ignored.
		//
		// The Switch module currently uses the same pins as the i2c bus,
		// as well as a number of the PWM pins (D5, 6).
		// This means that it excludes all modules but the MotionModule and
		// those using the UART.
		// (Above copied from node firmware source)
		out += "\"modules\": { ";
		out += "\"THP\": ";
		(nodes[i].modules & 0x01) ? out += "true, " : out += "false, ";
		out += "\"CO2\": ";
		(nodes[i].modules & 0x02) ? out += "true, " : out += "false, ";
		out += "\"Jura\": ";
		(nodes[i].modules & 0x04) ? out += "true, " : out += "false, ";
		out += "\"JuraTerm\": ";
		(nodes[i].modules & 0x08) ? out += "true, " : out += "false, ";
		out += "\"Motion\": ";
		(nodes[i].modules & 0x10) ? out += "true, " : out += "false, ";
		out += "\"PWM\": ";
		(nodes[i].modules & 0x20) ? out += "true, " : out += "false, ";
		out += "\"IO\": ";
		(nodes[i].modules & 0x40) ? out += "true, " : out += "false, ";
		out += "\"Switch\": ";
		(nodes[i].modules & 0x80) ? out += "true, " : out += "false, ";
		out += "\"Plant\": ";
		(nodes[i].modules & 0x100) ? out += "true" : out += "false";
		
		out += "} }";
		
		if ((i + 1) < nodes.size()) { out += ", "; }
	}
	
	out += "]";
	
	return out;
}


// --- UNASSIGNED TO JSON ---
// Returns a JSON array containing unassigned nodes.
// TODO: Buffer the output.
std::string Nodes::unassignedToJson() {
	std::string out = "[ ";
	
	for (int i = 0; i < newNodes.size(); ++i) {
		out += "{ \"uid\": \"" + newNodes[i].uid + "\", ";
		out += "\"location\": \"" + newNodes[i].location + "\" }";
		
		if ((i + 1) < newNodes.size()) { out += ", "; }
	}
	
	out += "]";
	
	return out;
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
bool Nodes::setTargetTemperature(std::string uid, float temp) {
	if (!initialized) { return false; }
	
	std::cout << "Setting target temperature for UID: " << uid << std::endl;
	
	Data::Statement update(*session);
	update << "UPDATE nodes SET target = ? WHERE uid = ?",
			use(temp),
			use(uid),
			now;
			
	return true;
}


// --- SET CURRENT TEMPERATURE ---
// Set a new current temperature for the specified node.
bool Nodes::setCurrentTemperature(std::string uid, float temp) {
	if (!initialized) { return false; }
	
	std::cout << "Updating current temperature for node " << uid << " to " 
			<< temp << std::endl;
	
	Data::Statement update(*session);
	update << "UPDATE nodes SET current = ? WHERE uid = ?",
			use(temp),
			use(uid),
			now;
			
	return true;
}


// --- SET DUTY ---
// Update the duty for the specified node.
bool Nodes::setDuty(std::string uid, uint8_t ch0, uint8_t ch1, uint8_t ch2, uint8_t ch3) {
	if (!initialized) { return false; }
	
	std::cout << "Setting duty for UID: " << uid << std::endl;
	
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
bool Nodes::setValves(std::string uid, bool ch0, bool ch1, bool ch2, bool ch3) {
	if (!initialized) { return false; }
	
	std::cout << "Setting valve state for UID: " << uid << std::endl;
	
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
bool Nodes::setSwitch(std::string uid, bool state) {
	if (!initialized) { return false; }
	
	std::cout << "Setting switch state for UID: " << uid << " to " << state << std::endl;
	
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
	
	std::cout << "Updating current temperatures..." << std::endl;
	
	// Get the UIDs.
	std::vector<std::string> uids;
	if (!getUIDs(uids)) { 
		std::cerr << "UpdateCurrentTemperatures: Failed to get the UIDs." << std::endl;
		return; 
	}
	
	std::cout << "Contacting Influx database..." << std::endl;
	
	int uidsl = uids.size();
	std::string responseStr;
	for (unsigned int i = 0; i < uidsl; ++i) {
		// Send message.
		std::string query = "SELECT%20\"value\"%20FROM%20\"temperature\"%20WHERE%20\"location\"='"
							+ uids[i] + "'%20ORDER%20BY%20time%20DESC%20LIMIT%201";
		try {			
			HTTPRequest request(HTTPRequest::HTTP_GET, "/query?db=" + influxDb + 
										"&q=" + query, HTTPMessage::HTTP_1_1);
										
			//std::cout << "Request: " << request.getURI() << endl;
												
			influxClient->sendRequest(request);
			
			HTTPResponse response;
			std::istream &rs = influxClient->receiveResponse(response);
			HTTPResponse::HTTPStatus status = response.getStatus();
			if (status != HTTPResponse::HTTP_OK/*  && 
					status != HTTPResponse::HTTP_NO_CONTENT */) {
				std::cerr << "Received InfluxDB error: " << response.getReason() << "\n";
				std::cerr << "Aborting temperature update loop.\n";
				break;
			}
			
			StreamCopier::copyToString(rs, responseStr);
		}
		catch (Exception& exc) {
			std::cout << "Exception caught while attempting to connect." << std::endl;
			std::cerr << exc.displayText() << std::endl;
			return;
		}
		
		//cout << "Influx response: " << responseStr << endl;
		
		// Process response.
		// Write the obtained temperature for each node to the database.
		Parser parser;
		Dynamic::Var result = parser.parse(responseStr);
		Object::Ptr object = result.extract<Object::Ptr>();
		if (object->has("error")) { // Error occurred.
			std::string err = object->getValue<std::string>("error");
			std::cerr << "Error from Influx DB for current temperature: " << err << std::endl;
			continue;
		}
		
		if (!object->isArray("results")) {
			std::cerr << "Results content wasn't an array. Skipping.\n";
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
	
	std::cout << "Calling Listener::checkNodes().\n";
	
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
bool Nodes::getUIDs(std::vector<std::string> &uids) {
	if (!initialized) { return false; }
	
	std::cout << "Getting UIDs...\n";
	
	uids.clear();
	
	//std::vector<string> temp;
	
	Data::Statement select(*session);
	std::string uid;
	select << "SELECT uid FROM nodes", into (uid), range(0, 1);
	//select << "SELECT uid FROM nodes", into (temp);
	
	while (!select.done()) {
		select.execute();
		std::cout << "Found UID: " << uid << std::endl;
		uids.push_back(uid);
	}
	
	//size_t rows = select.execute();
	//std::cout << "Rows: " << rows << endl;
	
	std::cout << "Found " << uids.size() << " nodes.\n";
	
	//uids = temp;
	
	return true;
}


// --- GET SWITCH UIDS ---
bool Nodes::getSwitchUIDs(std::vector<std::string> &uids) {
	if (!initialized) { return false; }
	
	std::cout << "Getting switch UIDs...\n";
	
	uids.clear();
	
	Data::Statement select(*session);
	std::string uid;
	select << "SELECT uid FROM switches", into (uid), range(0, 1);
	
	while (!select.done()) {
		select.execute();
		std::cout << "Found switch UID: " << uid << std::endl;
		uids.push_back(uid);
	}
	
	std::cout << "Found " << uids.size() << " nodes.\n";
	
	return true;
}
