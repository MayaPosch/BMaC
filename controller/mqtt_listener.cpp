/*
	mqtt_listener.cpp - MQTT listener of the BMaC controller.
	
	Revision 0.
	
	2022/07/29, Maya Posch
*/


#include "mqtt_listener.h"
#include "nodes.h"
#include "influx_client.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/File.h>

using namespace Poco::Data::Keywords;


// Structs
struct Node {
	std::string uid;
	std::string location;
	uint32_t modules;
	float posx;
	float posy;
};


// --- CONSTRUCTOR ---
Listener::Listener() {
	// Initialise the MQTT client.
	//client.setClientId("BMaC_Controller");
	using namespace std::placeholders;
	client.init(std::bind(&Listener::logHandler, this, _1, _2), NYMPH_LOG_LEVEL_TRACE);
	client.setMessageHandler(std::bind(&Listener::messageHandler, this, _1, _2, _3));
	
	//int keepalive = 60;
	//connect(host.c_str(), port, keepalive);
	
	// Set up SQLite database link.
	/* Data::SQLite::Connector::registerConnector();
	session = new Poco::Data::Session("SQLite", "nodes.db"); */
	
	// Ensure the database has a valid nodes table.
	/* (*session) << "CREATE TABLE IF NOT EXISTS nodes (uid TEXT UNIQUE, \
		location TEXT, \
		modules INT, \
		posx FLOAT, \
		posy FLOAT)", now; */
		
	// Ensure we have a valid firmware table.
	/* (*session) << "CREATE TABLE IF NOT EXISTS firmware (uid TEXT UNIQUE, \
		file TEXT)", now; */

	// Load configuration settings.
	this->defaultFirmware = defaultFirmware;
}


// --- DECONSTRUCTOR ---
Listener::~Listener() {
	client.shutdown();
}


// --- LOG HANDLER ---
void Listener::logHandler(int level, std::string text) {
	std::cout << level << " - " << text << std::endl;
}


// -- INIT ---
bool Listener::init(std::string clientId, std::string host, int port) {
	this->host = host;
	this->port = port;
	client.setClientId(clientId);
	
	return true;
}


// --- ADD SUBSCRIPTION ---
bool Listener::addSubscription(std::string topic) {
	std::string result;
	if (!client.subscribe(handle, topic, result)) {
		return false;
	}
	
	return true;
}


// --- CONNECT BROKER ---
bool Listener::connectBroker() {
	std::string result;
	if (!client.connect(host, port, handle, 0, conn, result)) {
		return false;
	}
	
	return true;
}


// --- DISCONNECT BROKER ---
bool Listener::disconnectBroker() {
	std::string result;
	client.disconnect(handle, result);
	
	return true;
}


// --- PUBLISH MESSAGE ---
bool Listener::publishMessage(std::string topic, std::string msg, uint8_t qos, bool retain) {
	MqttQoS qosLvl = MQTT_QOS_AT_MOST_ONCE;
	if (qos == 1) { qosLvl = MQTT_QOS_AT_LEAST_ONCE; }
	if (qos == 2) { qosLvl = MQTT_QOS_EXACTLY_ONCE; }
	std::string result;
	if (!client.publish(handle, topic, msg, result, qosLvl, retain)) {
		return false;
	}
	
	return true;
}


// --- MESSAGE HANDLER ---
void Listener::messageHandler(int handle, std::string topic, std::string payload) {
	// Handle:
	// * On Connect.
	// * Subscriptions.
	if (topic == "cc/config") {
		if (payload.length() < 1 || payload.length() != 12) {
			// Invalid payload. Reject.
			std::cerr << "Invalid payload: " << payload << ". Reject." << std::endl;
			return;
		}
		
		// TODO: Get the modules configuration for the specified node ID.
		// TODO: Handle an unknown node.
		NodeInfo node;
		//node.uid = payload;
		bool res = Nodes::getNodeInfo(payload, node);
		//node.modules = Nodes::getNodeModules(payload);
		//node.location = Nodes::getNodeLocation(payload);
		
		// Payload should be the UID of the node. Retrieve the configuration for
		// this UID and publish it on 'cc/<UID>' with the configuration as payload.
		/* Data::Statement select(*session);
		Node node;
		node.uid = payload;
		select << "SELECT location, modules FROM nodes WHERE uid=?",
				into (node.location),
				into (node.modules),
				use (payload);
				
		size_t rows = select.execute(); */
		
		// Send result.
		//if (rows == 1) {
		if (res) {
			std::string topic = "cc/" + payload;
			std::string response = "mod;" + std::string((const char*) &node.modules, 4);
			//publish(0, topic.c_str(), response.length(), response.c_str());
			publishMessage(topic, response);
			response = "loc;" + node.location;
			//publish(0, topic.c_str(), response.length(), response.c_str());
			publishMessage(topic, response);
		}
		//else if (rows < 1) {
		else {
			// No node with this UID found.
			std::cout << "No data found for uid " << payload << ". Added as new node." << std::endl;
		}/* 
		else {
			// Multiple data sets were found, which shouldn't be possible...
			std::cerr << "Error: Multiple data sets found for uid " << payload << std::endl;
		} */
	}
	else if (topic == "cc/ui/config") {
		// Payload is the desired resource to return:
		// * 'map'		- The layout image indicating node positioning.
		// * 'nodes'	- UID & position info (x/y) for each node.
		/* if (payload == "map") {
			// The map is expected to exist in the executable's folder (./).
			// Its name is 'map', with or without extension. If multiple files
			// with the name exist, the first one is taken.
			// It can be any image format. The client will identify it by its
			// binary signature (file header) and use it if compatible.
			// TODO: make map file configurable.
			std::ifstream mapFile("map.png", std::ios::binary);
			if (!mapFile.is_open()) {
				std::cerr << "Failed to open map file.\n";
				return;
			}
			
			// Read map file into string and publish it.
			// Topic: 'cc/ui/config/map'.
			std::stringstream ss;
			ss << mapFile.rdbuf();
			std::string mapData = ss.str();
			//publish(0, "cc/ui/config/map", mapData.length(), mapData.c_str());
			publishMessage("cc/ui/config/map", mapData);
		}
		else if (payload == "nodes") {
			// Check the number of rows.
			Data::Statement countQuery(*session);
			int rowCount;
			countQuery << "SELECT COUNT(*) FROM nodes",
				into(rowCount),
				now;
				
			if (rowCount == 0) {
				// We got nothing to send here. Return.
				std::cout << "No nodes found in database, returning..." << std::endl;
				return;
			}
			
			// Read the node info out of the database and create the string to
			// send.
			// Topic: 'cc/ui/config/nodes'.
			Data::Statement select(*session);
			Node node;
			select << "SELECT uid, location, modules, posx, posy FROM nodes",
					into (node.uid),
					into (node.location),
					into (node.modules),
					into (node.posx),
					into (node.posy),
					range(0, 1);
					
			// The string of nodes is the same format as for a new node (see 
			// below), except for this header:
			// uint64	total message size following this integer.
			// uint8(5)	"NODES" in ASCII
			// uint32	Number of node segments
			// <node segments>
			std::string header;
			std::string nodes;
			std::string nodeStr;
			uint32_t nodeCount = 0;
			while (!select.done()) {
				select.execute();
				nodeStr = "NODE";
				uint8_t length = (uint8_t) node.uid.length();
				nodeStr += std::string((char*) &length, 1);
				nodeStr += node.uid;
				length = (uint8_t) node.location.length();
				nodeStr += std::string((char*) &length, 1);
				nodeStr += node.location;
				nodeStr += std::string((char*) &node.posx, 4);
				nodeStr += std::string((char*) &node.posy, 4);
				nodeStr += std::string((char*) &node.modules, 4);
				uint32_t segSize = nodeStr.length();
				
				nodes += std::string((char*) &segSize, 4);
				nodes += nodeStr;
				++nodeCount;
			}
			
			// Complete header and append node segments.
			uint64_t messageSize = nodes.length() + 9; // ASCII string & node count
			header = std::string((char*) &messageSize, 8);
			header += "NODES";
			header += std::string((char*) &nodeCount, 4);
			header += nodes;
			
			//publish(0, "cc/nodes/all", header.length(), header.c_str());
			publishMessage("cc/nodes/all", header);
		} */
	}
	else if (topic == "cc/nodes/new") {
		// The payload should contain the information to create a new node.
		// Payload format (LE direction):
		// uint32	size of the segment after this integer.
		// uint8(4)	"NODE" in ASCII
		// uint8	UID length
		// uint8(*)	UID string
		// uint8	location length
		// uint8(*)	location string
		// float32	Position X
		// float32	Position Y
		// uint32	Active modules (bit flags)
		//
		// Module bitflags:
		// 0x1	- Temp/Humidity
		// 0x2	- CO2
		// 0x4	- Jura
		// 0x8	- JuraTerm
		
		/* uint32_t index = 0;
		uint32_t msgLength = *((uint32_t*) payload.substr(index, 4).data());
		index += 4;
		std::string signature = payload.substr(index, 4);
		index += 4;
		
		if (signature != "NODE") {
			std::cerr << "Invalid node signature." << std::endl;
			return;
		}
		
		UInt8 uidLength = (uint8_t) payload[index++];
		Node node;
		node.uid = payload.substr(index, uidLength);
		index += uidLength;
		uint8_t locationLength = (uint8_t) payload[index++];
		node.location = payload.substr(index, locationLength);
		index += locationLength;
		node.posx = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.posy = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.modules = *((uint32_t*) payload.substr(index, 4).data());
		
		// Debug
		std::cout << "Storing new node for UID: " << node.uid << std::endl;;
		
		// Store the node object in the database.
		Data::Statement insert(*session);
		insert << "INSERT INTO nodes VALUES(?, ?, ?, ?, ?)",
				use(node.uid),
				use(node.location),
				use(node.modules),
				use(node.posx),
				use(node.posy),
				now;
				
		// Store node UID with default firmware name as well.
		(*session) << "INSERT INTO firmware VALUES(?, ?)",
				use(node.uid),
				use(defaultFirmware),
				now; */
	}
	else if (topic == "cc/nodes/update") {
		// Update a single node.
		// The payload contains the usual node info. Deserialise it and store it
		// in the database.
		/* uint32_t index = 0;
		uint32_t msgLength = *((uint32_t*) payload.substr(index, 4).data());
		index += 4;
		std::string signature = payload.substr(index, 4);
		index += 4;
		
		if (signature != "NODE") {
			std::cerr << "Invalid node signature." << std::endl;
			return;
		}
		
		uint8_t uidLength = (uint8_t) payload[index++];
		Node node;
		node.uid = payload.substr(index, uidLength);
		index += uidLength;
		uint8_t locationLength = (uint8_t) payload[index++];
		node.location = payload.substr(index, locationLength);
		index += locationLength;
		node.posx = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.posy = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.modules = *((uint32_t*) payload.substr(index, 4).data());
		
		// Debug
		std::cout << "Updating node for UID: " << node.uid << std::endl;
		
		// Store the node object.
		Data::Statement update(*session);
		update << "UPDATE nodes SET location = ?, posx = ?, posy = ?, modules = ? WHERE uid = ?",
				use(node.location),
				use(node.posx),
				use(node.posy),
				use(node.modules),
				use(node.uid),
				now; */
	}
	else if (topic == "cc/nodes/delete") {
		// Delete the node with the specified UID.
		// Payload is the UID to delete.
		/* std::cout << "Deleting node with UID: " << payload << std::endl;
		
		Data::Statement del(*session);
		del << "DELETE FROM nodes WHERE uid = ?",
				use(payload),
				now;
				
		(*session) << "DELETE FROM firmware WHERE uid = ?",
				use(payload),
				now; */
	}
	else if (topic == "nsa/events/co2") {
		// CO2-related events. Currently hard-coded triggers in the node 
		// firmware:
		// If we stay above 850 ppm for 10 samples, send a warning. 
		// Above 1000 ppm an error. 
		// When dropping back below 750 ppm, send an 'OK'.
		//
		// Send a JSON-encoded message to the below HTTP URL:
		// curl -XPOST -H 'Content-Type: application/json' -d '{"state":"ok"}' 'https://<server>'`
		//
		// States:
		// * ok
		// * warn
		// * crit
		/* StringTokenizer st(payload, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
		if (st.count() < 4) {
			std::cerr << "CO2 event: Wrong number of arguments. Payload: " << payload << std::endl;
			return; 
		}
		
		std::string state = "ok";
		if (st[1] == "1") { state = "warn"; }
		else if (st[1] == "2") { state = "crit"; }
		std::string increase = (st[2] == "1") ? "true" : "false";
		std::string json = "{ \"state\": \"" + state + "\", \
						\"location\": \"" + st[0] + "\", \
						\"increase\": " + increase + ", \
						\"ppm\": " + st[3] + " }";
						
		// Send the JSON string to the URL.
		Net::HTTPSClientSession httpsClient("localhost");
		try {
			Net::HTTPRequest request(Net::HTTPRequest::HTTP_POST, 
									"/", 
									Net::HTTPMessage::HTTP_1_1);
			request.setContentLength(json.length());
			request.setContentType("application/json");
			httpsClient.sendRequest(request) << json;
			
			Net::HTTPResponse response;
			httpsClient.receiveResponse(response);
		}
		catch (Exception& exc) {
			std::cout << "Exception caught while attempting to connect." << std::endl;
			std::cerr << exc.displayText() << std::endl;
			return;
		} */
	}
	else if (topic == "cc/firmware") {
		if (payload == "list") {
			// Return a list of the available firmware images as found on the 
			// filesystem.
			std::vector<File> files;
			File file("firmware");
			if (!file.isDirectory()) { return; }
			
			file.list(files);
			std::string out;
			for (int i = 0; i < files.size(); ++i) {
				if (files[i].isFile()) {
					out += files[i].path();
					out += ";";
				}
			}
			
			// Erase last semi-colon.
			out.pop_back();
			
			//publish(0, "cc/firmware/list", out.length(), out.c_str());
			publishMessage("cc/firmware/list", out);
		}
		else {
			// Payload should contain the command followed by any further data.
			// All separated by semi-colons.
			StringTokenizer st(payload, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
			
			if (st[0] == "change") {
				// Change the assigned firmware for a UID from the default.
				// Second argument should be UID, third the new firmware name.
				// TODO: add check for whether new firmware file exists?
				if (st.count() != 3) { return; }
				(*session) << "UPDATE firmware SET file = ? WHERE uid = ?",
								use (st[1]),
								use (st[2]),
								now;
			}
			else if (st[0] == "upload") {
				// Save the new firmware data to disk. Overwrites an existing 
				// file with the same name.
				// Second argument is filename, third argument is file data.
				// TODO: add some kind of CRC check.
				if (st.count() != 3) { return; }
				
				// Write file & truncate if exists.
				std::string filepath = "firmware/" + st[1];				
				std::ofstream outfile("firmware/" + st[1], std::ofstream::binary | std::ofstream::trunc);
				outfile.write(st[2].data(), st[2].size());
				outfile.close();
			}
		}
	}
	else if (topic == "pwm/response") {
		// Payload is the node UID (string) followed by a semi-colon and the
		// rest of the payload.
		StringTokenizer st(payload, ";");
		if (st.count() < 2) {
			std::cerr << "PWM message: Wrong number of arguments. Payload: " << payload << "\n";
			return; 
		}
		
		//std::cout << "Payload: " << payload << std::endl;
		//std::cout << "Binary: " << std::hex << *((uint32_t*) st[1].c_str()) << std::endl;
		
		std::string uid = st[0];
		
		// TODO: check that this node UID is known.
		
		/* nodesLock.lock();
		std::map<std::string, NodeInfo>::iterator it;
		it = nodes.find(uid);
		if (it == nodes.end()) {
			std::cerr << "Unknown UID. Skipping.\n";
			nodesLock.unlock();
			return;
		} */
		
		/* std::cout << "PWM receive for UID " << uid << ", validate: " << 
									(uint32_t) it->second.validate << std::endl;
		
		if (it->second.validate == 0) {
			// We should have received a list of the active pins:
			// <uint8*>
			std::string pins = st[1];
			
			// If the list is empty, the node needs to be initialised.
			// For now we assume three connected channels (0xc, 0xd, 0xe).
			// TODO: use saved channel info for each node for intelligent queries.
			if (pins.empty()) {
				std::cout << "Initializing UID: " << uid << std::endl;
				
				std::string topic = "pwm/" + uid;
				char initAll[] = { 0x01, 0x03, 0x0c, 0x0d, 0x0e };
				//publish(0, topic.c_str(), 5, initAll, 1); // QoS 1.
				publishMessage(topic, initAll, 1); // QoS 1.
				
				// Pause for half a second while the node initialises.
				Thread::sleep(500);
			}
			//else if (st[1].compare(GPIOPins) == 0) {
			else if (pins.length() == 3) {
				// All pins are active. We can continue.
				it->second.validate = 1;
				
				// Set state of channel validation level.
				it->second.ch0_valid = false;
				it->second.ch1_valid = false;
				it->second.ch2_valid = false;
				it->second.ch3_valid = false;
				
				// Request the duty cycle for each active pin.
				std::string topic = "pwm/" + uid;
				char ch0duty[] = { 0x08, 0x0c };
				char ch1duty[] = { 0x08, 0x0d };
				char ch2duty[] = { 0x08, 0x0e };
				char ch3duty[] = { 0x08, 0x0f };
				//publish(0, topic.c_str(), 2, ch0duty, 1); // QoS 1
				//publish(0, topic.c_str(), 2, ch1duty, 1); // QoS 1
				//publish(0, topic.c_str(), 2, ch2duty, 1); // QoS 1
				//publish(0, topic.c_str(), 2, ch3duty, 1); // QoS 1
				publishMessage(topic, ch0duty, 1); // QoS 1.
				publishMessage(topic, ch1duty, 1); // QoS 1.
				publishMessage(topic, ch2duty, 1); // QoS 1.
				publishMessage(topic, ch3duty, 1); // QoS 1.
			}
			else {				
				// The node should be initialised now. 
				// Look for the '1' confirmation.
				if (st[1] != "1") {
					std::cerr << "Initializing node '" + uid + "' failed. Skipping.\n";
					//nodesLock.unlock();
					return;
				}
				
				// Resend the request for the active pins.
				std::string topic = "pwm/" + uid;
				char payload[] = { 0x10 };
				//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
				publishMessage(topic, payload, 1); // QoS 1.
			}
		}
		
		if (it->second.validate == 1) {
			// The node is active, but we need to validate the state of its
			// PWM outputs.
			// Here we receive answers from nodes containing the current duty 
			// for a specific pin:
			// uint8	GPIO number.
			// uint8	Duty level (1 - 6).
			//
			// Duty level returned from the node is the set level + 1 for 
			// technical reasons. Subtract one when comparing with the NodeInfo
			// data.
			std::string res = st[1];
			if (res.length() != 2) {
				std::cerr << "Received wrong response length from node for duty level. Skipping.\n";
				//nodesLock.unlock();
				return;
			}
			
			uint8_t ch = (uint8_t) res[0];
			int count = 0;
			if (it->second.ch0_valid) { ++count; }
			if (it->second.ch1_valid) { ++count; }
			if (it->second.ch2_valid) { ++count; }
			if (it->second.ch3_valid) { ++count; }
			
			std::cout << "Validating channel: " << std::hex << (uint32_t) ch << std::endl;
			
			std::string topic = "pwm/" + uid;
			if (count == 4) {
				// We're done.
				it->second.validate = 2;
			}
			else if (ch == 0x0c) { // ch1
				// Validate this channel.
				uint8_t duty = (uint8_t) res[1];
				if ((duty - 1) == it->second.ch1_duty) { it->second.ch1_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					//unsigned char payload[] = { 0x04, 0x0c, it->second.ch1_duty };
					std::string payload({ 0x04, 0x0c, it->second.ch1_duty });
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1
					it->second.ch1_valid = true;
					++count;
				}
			}
			else if (ch == 0x0d) { // ch2
				// Validate this channel.
				uint8_t duty = (uint8_t) res[1];
				if ((duty - 1) == it->second.ch2_duty) { it->second.ch2_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					//unsigned char payload[] = { 0x04, 0x0d, it->second.ch1_duty };
					std::string payload({ 0x04, 0x0d, it->second.ch2_duty });
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
					it->second.ch2_valid = true;
					++count;
				}				
			}
			else if (ch == 0x0e) { // ch0
				// Validate this channel.
				uint8_t duty = (uint8_t) res[1];
				if ((duty - 1) == it->second.ch0_duty) { it->second.ch0_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					//unsigned char payload[] = { 0x04, 0x0e, it->second.ch0_duty };
					std::string payload({ 0x04, 0x0e, it->second.ch0_duty });
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
					it->second.ch0_valid = true;
					++count;
				}
			}
			else if (ch == 0x0f) { // ch3
				// Validate this channel.
				uint8_t duty = (uint8_t) res[1];
				if ((duty - 1) == it->second.ch3_duty) { it->second.ch3_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					//unsigned char payload[] = { 0x04, 0x0f, it->second.ch3_duty };
					std::string payload({ 0x04, 0x0f, it->second.ch3_duty });
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
					it->second.ch3_valid = true;
					++count;
				}
			}
			
			if (count == 4) {
				// We're done.
				it->second.validate = 2;
			}
		}
		
		if (it->second.validate == 2) {
			// Node has been validated. Adjust the settings for the node based
			// on the target & current temperature.
			//
			// The measured (current) temperature is measured at the ceiling.
			// To compensate for this we subtract 1C from it.
			float delta = (it->second.current - 1) - it->second.target;
			
			// Our delta tells us whether it's too warm (positive value), or
			// too cold (negative value). This tells us which channels to change.
			//
			// If we're cooling, we need to invert the delta to get the 
			// appropriate response.
			if (heating) {
				delta = delta * -1;
			}
			
			std::cout << "Current temperature delta: " << delta << std::endl;
			
			//
			// An important factor here is whether the AC unit (fan coil unit, 
			// FCU) is set to cool or heat. If it's too warm, but the system is 
			// set to heating, then there's nothing we can do with the fans.
			//
			// We can switch the entire system in a section from heating to 
			// cooling using the appropriate switch.
			
			//
			if (delta > 4.0) {
				// All to level 5.
				Nodes::setDuty(uid, 5, 5, 5, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 3.0) {
				// All to level 4.
				Nodes::setDuty(uid, 4, 4, 4, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 2.0) {
				// All to level 3.
				Nodes::setDuty(uid, 3, 3, 3, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 1.0) {
				// All to level 2.
				Nodes::setDuty(uid, 2, 2, 2, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 0.8) {
				// Open the valve on all units.
				//Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 0.6) {
				// All to level 1.
				Nodes::setDuty(uid, 1, 1, 1, 0);
				
				// All valves open.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 0.4) {
				// Channels 0 and 1 to level 1.
				Nodes::setDuty(uid, 1, 1, 0 , 0);
				
				// Open first two valves.
				Nodes::setValves(uid, true, true, false, false);
			}
			else if (delta > 0.2) {
				// Channel 1 ('center') to 1.
				Nodes::setDuty(uid, 0, 1, 0, 0);
				
				// Open center valve.
				Nodes::setValves(uid, false, true, false, false);
			}
			else if (delta < -3.0) {
				// Switch the system from the current mode (cooling/heating) to
				// the opposite mode in order to regain effectiveness.
				// FIXME: hard-coding switch ID. Make dynamic.
				Nodes::setSwitch("sw-grossraum", !heating);
				checkSwitch();
			}
			else {
				// All fans off.
				Nodes::setDuty(uid, 0, 0, 0, 0);
				
				// Close all valves.
				Nodes::setValves(uid, false, false, false, false);
			}

			it->second.validate = 3;
		} */
			
		//nodesLock.unlock();
	}
	else if (topic.compare(0, 11, "io/response") == 0) {
		StringTokenizer st(payload, ";");
		if (st.count() < 2) {
			std::cerr << "I/O message: Wrong number of arguments. Payload: " 
					<< payload << "\n";
			return; 
		}
		
		std::string uid = st[0];
		
		// Check the command we get a response to and respond appropriately.
		if (st[1].length() < 2) {
			std::cerr << "I/O message: response with fewer than two parameters.\n";
			return;
		}
		
		uint8_t cmd = (uint8_t) st[1][0];
		if (cmd == 0x01) {	// Start.
			if (st[1][1] != 0x01) {
				std::cerr << "I/O: failed to start node " << uid << std::endl;
				return;
			}
			
			// Request the current state.
			std::string topic = "io/" + uid;
			char payload[] = { 0x04 };
			//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
			publishMessage(topic, payload, 1); // QoS 1.
		}
		else if (cmd == 0x02) { // Stop.
			// Nothing.
		}
		else if (cmd == 0x04) { // State.
			if (st[1].length() != 5) {
				std::cerr << "I/O: Received corrupted I/O state message response.\n";
				std::cerr << "I/O: Length of state message was " << st[1].length() 
						<< " bytes.\n";
				return;
			}
			
			valvesLock.lock();
			std::map<std::string, ValveInfo>::iterator it;
			it = valves.find(uid);
			if (it == valves.end()) {
				std::cerr << "Unknown UID. Skipping.\n";
				valvesLock.unlock();
				return;
			}
			
			// Compare the valve settings with the desired settings.
			std::string topic = "io/" + uid;
			uint8_t gpio = (uint8_t) st[1][4];
			if (it->second.ch0_valve) {
				// FIXME: we're just getting 0x00 back for each register at this
				// point; ignore the value for now and explicitly set the pin.
				//if (!(gpio & 0x1)) {
					char payload[] { 0x20, 0x01, 0x01 }; // Turn on valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x1)) {
					char payload[] { 0x20, 0x01, 0x00 }; // Turn off valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			
			if (it->second.ch1_valve) {
				//if (!(gpio & 0x2)) {
					char payload[] { 0x20, 0x02, 0x01 }; // Turn on valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x2)) {
					char payload[] { 0x20, 0x02, 0x00 }; // Turn off valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}	
			
			if (it->second.ch2_valve) {
				//if (!(gpio & 0x4)) {
					char payload[] { 0x20, 0x03, 0x01 }; // Turn on valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x4)) {
					char payload[] { 0x20, 0x03, 0x00 }; // Turn off valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}	
			
			if (it->second.ch3_valve) {
				//if (!(gpio & 0x8)) {
					char payload[] { 0x20, 0x04, 0x01 }; // Turn on valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x8)) {
					char payload[] { 0x20, 0x04, 0x00 }; // Turn off valve.
					//publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					publishMessage(topic, payload, 1); // QoS 1.
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}					
			
			valvesLock.unlock();
		}
		else if (cmd == 0x08) { // Set mode.
			// All pins are set to 'output' mode by default. Nothing to do here.
		}
		else if (cmd == 0x10) { // Pull-up.
			// Nothing.
		}
		else if (cmd == 0x20) { // Write.
			if (st[1][1] != 0x01) {
				std::cerr << "I/O: failed to write pin on node " << uid << std::endl;
				return;
			}
		}
		else if (cmd == 0x40) { // Read.
			// Nothing.
		}
		else if (cmd == 0x80) { // Status.
			// Active status response. If 0x0, activate.
			// If active (0x1), start validation of settings.
			if (st[1].length() == 2) {
				if (st[1][1] == 0x0) {
					std::cerr << "I/O: error requesting active status.\n";
					return;
				}
				else {
					std::cerr << "I/O: corrupted active status response.\n";
					std::cerr << "I/O: received: ";
					for (int i = 0; i < st[1].length(); ++i) {
						std::cerr << " "
							<< NumberFormatter::formatHex((uint8_t) st[1][i], true); 
					}
					
					std::cerr << std::endl;
					return;
				}
			}
			else if (st[1].length() == 3) {
				if (st[1][1] != 0x01) {
					std::cerr << "I/O: active status reported failure.\n";
					return;
				}
				
				if (st[1][2] == 0x0) {
					// Initialise.
					std::string topic = "io/" + uid;
					char payload[] = { 0x01 };
					//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
					publishMessage(topic, payload, 1); // QoS 1.
				}
				else if (st[1][2] == 0x01) {
					// Validate.
					std::string topic = "io/" + uid;
					char payload[] = { 0x04 };
					//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
					publishMessage(topic, payload, 1); // QoS 1.
				}
				else {
					std::cerr << "I/O: invalid active status response value.\n";
					return;
				}
			}
		}
		else {
			std::cerr << "I/O message: unknown command.\n";
			return;
		}
	}
	else if (topic.compare(0, 15, "switch/response") == 0) {
		// We receive either the current position of the switch here, or a 
		// success/failure message for the changing of the switch's position.
		StringTokenizer st(payload, ";");
		if (st.count() < 2) {
			std::cerr << "Switch message: Wrong number of arguments. Payload: " << payload << "\n";
			return; 
		}
		
		std::string uid = st[0];
		
		// FIXME: currently we assume that we just have a single switch and thus
		// a single heating/cooling status. 
		// Read out payload value and set global variable.
		if (st[1].length() < 2) {
			std::cerr << "Switch message: response with fewer than two parameters.\n";
			return;
		}
		
		switchesLock.lock();
		std::map<std::string, SwitchInfo>::iterator it;
		it = switches.find(uid);
		if (it == switches.end()) {
			std::cerr << "Unknown UID. Skipping.\n";
			switchesLock.unlock();
			return;
		}
		
		if (st[1][0] == 0x04) {
			// Response containing the currently active pin.
			if (st[1].length() != 3) {
				std::cerr << "Switch message: wrong number of parameters for state response.";
				switchesLock.unlock();
				return;
			}
			
			// Validate switch state.
			uint8_t pin = (uint8_t) st[1][2];
			if (pin == 0x00) { // Cooling state.
				if (it->second.state) {
					// Switch to heating.
					std::string topic = "switch/" + uid;
					char payload[] = { 0x02 };
					//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
					publishMessage(topic, payload, 1); // QoS 1.
				}
				else {
					std::cout << "Switch: confirming status as 'cooling'\n";
					heating = false;
				}
			}
			else if (pin == 0x01) { 
				if (!(it->second.state)) {
					// Switch to cooling.
					std::string topic = "switch/" + uid;
					char payload[] = { 0x01 };
					//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
					publishMessage(topic, payload, 1); // QoS 1.
				}
				else {
					std::cout << "Switch: confirming status as 'heating'\n";
					heating = true;
				}
			}
			else {
				std::cerr << "Switch message: received invalid switch state." << std::endl;
				switchesLock.unlock();
				return; 
			}
			
		}
		else if (st[1][0] == 0x01) {
			// Switch 1 (cooling position). Check return value.
			if (st[1][1] != 0x01) {
				// Command didn't succeed.
				switchesLock.unlock();
				return;
			}
			
			std::cout << "Switch: setting status to 'cooling'\n";
			heating = false;
		}
		else if (st[1][0] == 0x02) {
			// Switch 2 (heating position). Check return value.
			if (st[1][1] != 0x01) {
				// Command didn't succeed.
				switchesLock.unlock();
				return;
			}
			
			std::cout << "Switch: setting status to 'heating'\n";
			heating = true;
		}
		
		switchesLock.unlock();
	}
	else {
		// Assume possible MQTT-To-Influx topic. Check topics.
		std::map<std::string, std::string>::iterator it = series.find(topic);
		if (it == series.end()) { 
			std::cerr << "Topic not found: " << topic << "\n";
			return; 
		}
	
		if (payload.length() < 1) {
			std::cerr << "No payload found. Returning...\n";
			return;
		}
		
		size_t pos = payload.find(";");
		if (pos == std::string::npos || pos == 0) {
			// Invalid payload. Reject.
			std::cerr << "Invalid payload: " << payload << ". Reject.\n";
			return;
		}
		
		// Assemble the message to send to the InfluxDB instance. 
		// This message contains the uid and value from the MQTT payload, as well
		// as information from the listener's configuration.
		// Note: The timestamp is currently added by the InfluxDB, which is why it's
		// commented out here.
		// TODO: is a space (0x20) a valid UID?
		std::string uid = payload.substr(0, pos);
		std::string value = payload.substr(pos + 1);
		std::string influxMsg;	
		influxMsg = series[topic];
		influxMsg += ",location=" + uid;
		influxMsg += " value=" + value;
		//influxMsg += " " + to_string(static_cast<long int>(time(0)));
		
		// Send message.
		/* try {
			Net::HTTPRequest request(Net::HTTPRequest::HTTP_POST, "/write?db=" + influxDb, Net::HTTPMessage::HTTP_1_1);
			request.setContentLength(influxMsg.length());
			request.setContentType("application/x-www-form-urlencoded");
			influxClient->sendRequest(request) << influxMsg;
			
			Net::HTTPResponse response;
			influxClient->receiveResponse(response);
			/* Poco::Net::HTTPResponse::HTTPStatus status = response.getStatus();
			if (status != Poco::Net::HTTPResponse::HTTP_OK && status != Poco::Net::HTTPResponse::HTTP_NO_CONTENT) {
				cerr << "Received InfluxDB error: " << response.getReason() << "\n";
			} *//*
		}
		catch (Exception& exc) {
			std::cout << "Exception caught while attempting to connect." << std::endl;
			std::cerr << exc.displayText() << std::endl;
			return;
		} */
		
		InfluxClient::writeQuery(influxMsg);
	}
}


// --- CHECK NODES ---
// Check the PWM and I/O status for each node: active pins, current duty.
// Adjust active pins and duty cycle as needed.
bool Listener::checkNodes() {
	std::cout << "Listener::checkNodes() called." << std::endl;
	
	// Get the info on each node.
	std::vector<std::string> uids;
	if (!Nodes::getUIDs(uids)) { return false; }
	
	std::cout << "Checking nodes: " << uids.size() << std::endl;
	
	int uidsl = uids.size();
	//nodes.clear();
	//nodesLock.lock();
	for (unsigned int i = 0; i < uidsl; ++i) {
		// Start the validation sequence for this node.
		NodeInfo info;
		if (!Nodes::getNodeInfo(uids[i], info)) {
			std::cerr << "Error getting info for node " << uids[i] << ". Skipping..." << std::endl;
			continue;
		}
		
		ValveInfo vinfo;
		if (!Nodes::getValveInfo(uids[i], vinfo)) {
			std::cerr << "Error getting valve info for node " << uids[i] << 
													". Skipping..." << std::endl;
			continue;
		}
		
		// Store info for this node.
		info.validate = 0; // starting a new validation cycle.
		//nodes[uids[i]] = info;
		valves[uids[i]] = vinfo;
		
		// Request list of active pins from the node.
		std::string topic = "pwm/" + uids[i];
		char payload[] = { 0x10 };
		//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
		publishMessage(topic, payload);
		
		// Request current valve status from the node.
		topic = "io/" + uids[i];
		//unsigned char pl[] = { 0x80 };
		std::string pl({ (int8_t) 0x80 });
		//publish(0, topic.c_str(), 1, pl, 1);
		publishMessage(topic, pl);
	}
	
	//nodesLock.unlock();
	return true;
}


// --- CHECK SWITCH ---
// Request the status of the heating/cooling switch.
bool Listener::checkSwitch() {
	std::cout << "Listener::checkSwitch() called." << std::endl;
	
	// Get the info on each node.
	std::vector<std::string> uids;
	if (!Nodes::getSwitchUIDs(uids)) { return false; }
	
	int uidsl = uids.size();
	switchesLock.lock();
	for (unsigned int i = 0; i < uidsl; ++i) {
		SwitchInfo info;
		if (!Nodes::getSwitchInfo(uids[i], info)) {
			std::cerr << "Error getting info for switch " << uids[i] << ". Skipping..." << std::endl;
			continue;
		}
		
		// Store info for this switch.
		switches[uids[i]] = info;
		
		// Sync the system state with the stored state.
		heating = info.state;
		
		// Send status request to switch.
		std::string topic = "switch/" + uids[i];
		char payload[] = { 0x04 };
		//publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
		publishMessage(topic, payload, 1); // QoS 1.
	}
	
	switchesLock.unlock();
	
	return true;
}


// --- GET LOCAL IP ---
std::string Listener::getLocalIP() {
	return client.getLocalAddress(handle);
}
