/*
	listener.cpp - MQTT listener for the BMaC C&C server.
	
	Revision 0
	
	Features:
				- 
				
	Notes:
				-
				
	2017/02/09, Maya Posch <posch@synyx.de>
*/

#include "listener.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/File.h>

using namespace Poco::Data::Keywords;


// Structs
struct Node {
	string uid;
	string location;
	UInt32 modules;
	float posx;
	float posy;
};


// --- CONSTRUCTOR ---
Listener::Listener(string clientId, string host, int port, string defaultFirmware) : mosquittopp(clientId.c_str()) {
	int keepalive = 60;
	connect(host.c_str(), port, keepalive);
	
	// Set up SQLite database link.
	Data::SQLite::Connector::registerConnector();
	session = new Poco::Data::Session("SQLite", "nodes.db");
	
	// Ensure the database has a valid nodes table.
	(*session) << "CREATE TABLE IF NOT EXISTS nodes (uid TEXT UNIQUE, \
		location TEXT, \
		modules INT, \
		posx FLOAT, \
		posy FLOAT)", now;
		
	// Ensure we have a valid firmware table.
	(*session) << "CREATE TABLE IF NOT EXISTS firmware (uid TEXT UNIQUE, \
		file TEXT)", now;

	// Load configuration settings.
	this->defaultFirmware = defaultFirmware;
}


// --- DECONSTRUCTOR ---
Listener::~Listener() {
	//
}


// --- ON CONNECT ---
void Listener::on_connect(int rc) {
	cout << "Connected. Subscribing to topics...\n";
	
	// Check code.
	if (rc == 0) {
		// Subscribe to desired topics.
		string topic = "cc/config";	// announce by nodes coming online.
		subscribe(0, topic.c_str());
		topic = "cc/ui/config";		// C&C client requesting configuration.
		subscribe(0, topic.c_str());
		topic = "cc/nodes/new";		// C&C client adding new node.
		subscribe(0, topic.c_str());
		topic = "cc/nodes/update";	// C&C client updating node.
		subscribe(0, topic.c_str());
		topic = "nsa/events/co2";	// CO2-related events.
		subscribe(0, topic.c_str());
		topic = "cc/firmware";	// C&C client firmware command.
		subscribe(0, topic.c_str());
	}
	else {
		// handle.
		cerr << "Connection failed. Aborting subscribing.\n";
	}
}


// --- ON MESSAGE ---
void Listener::on_message(const struct mosquitto_message* message) {
	string topic = message->topic;
	string payload = string((const char*) message->payload, message->payloadlen);
	
	if (topic == "cc/config") {
		if (payload.length() < 1) {
			// Invalid payload. Reject.
			cerr << "Invalid payload: " << payload << ". Reject.\n";
			return;
		}
		
		// Payload should be the UID of the node. Retrieve the configuration for
		// this UID and publish it on 'cc/<UID>' with the configuration as payload.
		Data::Statement select(*session);
		Node node;
		node.uid = payload;
		select << "SELECT location, modules FROM nodes WHERE uid=?",
				into (node.location),
				into (node.modules),
				use (payload);
				
		size_t rows = select.execute();
		
		// Send result.
		if (rows == 1) {
			string topic = "cc/" + payload;
			string response = "mod;" + string((const char*) &node.modules, 4);
			publish(0, topic.c_str(), response.length(), response.c_str());
			response = "loc;" + node.location;
			publish(0, topic.c_str(), response.length(), response.c_str());
		}
		else if (rows < 1) {
			// No node with this UID found.
			cerr << "Error: No data set found for uid " << payload << endl;
		}
		else {
			// Multiple data sets were found, which shouldn't be possible...
			cerr << "Error: Multiple data sets found for uid " << payload << "\n";
		}
	}
	else if (topic == "cc/ui/config") {
		// Payload is the desired resource to return:
		// * 'map'		- The layout image indicating node positioning.
		// * 'nodes'	- UID & position info (x/y) for each node.
		if (payload == "map") {
			// The map is expected to exist in the executable's folder (./).
			// Its name is 'map', with or without extension. If multiple files
			// with the name exist, the first one is taken.
			// It can be any image format. The client will identify it by its
			// binary signature (file header) and use it if compatible.
			// TODO: make map file configurable.
			ifstream mapFile("map.png", ios::binary);
			if (!mapFile.is_open()) {
				cerr << "Failed to open map file.\n";
				return;
			}
			
			// Read map file into string and publish it.
			// Topic: 'cc/ui/config/map'.
			stringstream ss;
			ss << mapFile.rdbuf();
			string mapData = ss.str();
			publish(0, "cc/ui/config/map", mapData.length(), mapData.c_str());
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
				cout << "No nodes found in database, returning...\n";
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
			string header;
			string nodes;
			string nodeStr;
			UInt32 nodeCount = 0;
			while (!select.done()) {
				select.execute();
				nodeStr = "NODE";
				UInt8 length = (UInt8) node.uid.length();
				nodeStr += string((char*) &length, 1);
				nodeStr += node.uid;
				length = (UInt8) node.location.length();
				nodeStr += string((char*) &length, 1);
				nodeStr += node.location;
				nodeStr += string((char*) &node.posx, 4);
				nodeStr += string((char*) &node.posy, 4);
				nodeStr += string((char*) &node.modules, 4);
				UInt32 segSize = nodeStr.length();
				
				nodes += string((char*) &segSize, 4);
				nodes += nodeStr;
				++nodeCount;
			}
			
			// Complete header and append node segments.
			UInt64 messageSize = nodes.length() + 9; // ASCII string & node count
			header = string((char*) &messageSize, 8);
			header += "NODES";
			header += string((char*) &nodeCount, 4);
			header += nodes;
			
			publish(0, "cc/nodes/all", header.length(), header.c_str());
		}
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
		
		UInt32 index = 0;
		UInt32 msgLength = *((UInt32*) payload.substr(index, 4).data());
		index += 4;
		string signature = payload.substr(index, 4);
		index += 4;
		
		if (signature != "NODE") {
			cerr << "Invalid node signature.\n";
			return;
		}
		
		UInt8 uidLength = (UInt8) payload[index++];
		Node node;
		node.uid = payload.substr(index, uidLength);
		index += uidLength;
		UInt8 locationLength = (UInt8) payload[index++];
		node.location = payload.substr(index, locationLength);
		index += locationLength;
		node.posx = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.posy = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.modules = *((UInt32*) payload.substr(index, 4).data());
		
		// Debug
		cout << "Storing new node for UID: " << node.uid << "\n";
		
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
				now;
	}
	else if (topic == "cc/nodes/update") {
		// Update a single node.
		// The payload contains the usual node info. Deserialise it and store it
		// in the database.
		UInt32 index = 0;
		UInt32 msgLength = *((UInt32*) payload.substr(index, 4).data());
		index += 4;
		string signature = payload.substr(index, 4);
		index += 4;
		
		if (signature != "NODE") {
			cerr << "Invalid node signature.\n";
			return;
		}
		
		UInt8 uidLength = (UInt8) payload[index++];
		Node node;
		node.uid = payload.substr(index, uidLength);
		index += uidLength;
		UInt8 locationLength = (UInt8) payload[index++];
		node.location = payload.substr(index, locationLength);
		index += locationLength;
		node.posx = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.posy = *((float*) payload.substr(index, 4).data());
		index += 4;
		node.modules = *((UInt32*) payload.substr(index, 4).data());
		
		// Debug
		cout << "Updating node for UID: " << node.uid << "\n";
		
		// Store the node object.
		Data::Statement update(*session);
		update << "UPDATE nodes SET location = ?, posx = ?, posy = ?, modules = ? WHERE uid = ?",
				use(node.location),
				use(node.posx),
				use(node.posy),
				use(node.modules),
				use(node.uid),
				now;
	}
	else if (topic == "cc/nodes/delete") {
		// Delete the node with the specified UID.
		// Payload is the UID to delete.
		cout << "Deleting node with UID: " << payload << "\n";
		
		Data::Statement del(*session);
		del << "DELETE FROM nodes WHERE uid = ?",
				use(payload),
				now;
				
		(*session) << "DELETE FROM firmware WHERE uid = ?",
				use(payload),
				now;
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
		StringTokenizer st(payload, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
		if (st.count() < 4) {
			cerr << "CO2 event: Wrong number of arguments. Payload: " << payload << "\n";
			return; 
		}
		
		string state = "ok";
		if (st[1] == "1") { state = "warn"; }
		else if (st[1] == "2") { state = "crit"; }
		string increase = (st[2] == "1") ? "true" : "false";
		string json = "{ \"state\": \"" + state + "\", \
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
			cout << "Exception caught while attempting to connect." << std::endl;
			cerr << exc.displayText() << std::endl;
			return;
		}
	}
	else if (topic == "cc/firmware") {
		if (payload == "list") {
			// Return a list of the available firmware images as found on the 
			// filesystem.
			std::vector<File> files;
			File file("firmware");
			if (!file.isDirectory()) { return; }
			
			file.list(files);
			string out;
			for (int i = 0; i < files.size(); ++i) {
				if (files[i].isFile()) {
					out += files[i].path();
					out += ";";
				}
			}
			
			// Erase last semi-colon.
			out.pop_back();
			
			publish(0, "cc/firmware/list", out.length(), out.c_str());
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
				string filepath = "firmware/" + st[1];				
				ofstream outfile("firmware/" + st[1], ofstream::binary | ofstream::trunc);
				outfile.write(st[2].data(), st[2].size());
				outfile.close();
			}
		}
	}
}


// --- ON SUBSCRIBE ---
void Listener::on_subscribe(int mid, int qos_count, const int* granted_qos) {
	// Report success, with details.
}
