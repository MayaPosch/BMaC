/*
	influx_mqtt.cpp - Proxy to convert MQTT events into InfluxDB HTTP protocol.
	
	Revision 0
	
	Features:
				- 
				
	Notes:
				-
				
	2017/02/09, Maya Posch <posch@synyx.de>
*/

#include "mth.h"

//#include <ctime>
#include <iostream>

using namespace std;

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>

using namespace Poco;


// --- CONSTRUCTOR ---
MtH::MtH(string clientId, string host, int port, string topics, string influxHost, 
			int influxPort, string influxDb, string influx_sec) : mosquittopp(clientId.c_str()) {
	this->topics  = topics;
	this->influxDb = influxDb;
	if (influx_sec == "true") { 
		cout << "Connecting with HTTPS..." << std::endl;
		influxClient = new Net::HTTPSClientSession(influxHost, influxPort);
		secure = true; 
	} 
	else {
		cout << "Connecting with HTTP..." << std::endl;
		influxClient = new Net::HTTPClientSession(influxHost, influxPort);
		secure = false; 
	}
	
	int keepalive = 60;
	connect(host.c_str(), port, keepalive);
}


// --- DECONSTRUCTOR ---
MtH::~MtH() {
	delete influxClient;
}


// --- ON CONNECT ---
void MtH::on_connect(int rc) {
	cout << "Connected. Subscribing to topics...\n";
	
	// Check code.
	if (rc == 0) {
		// Subscribe to desired topics.
		StringTokenizer st(topics, ",", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
		for (StringTokenizer::Iterator it = st.begin(); it != st.end(); ++it) {
			string topic = string(*it);
			cout << "Subscribing to: " << topic << "\n";
			subscribe(0, topic.c_str());
			
			// Add name of the series to the 'series' map.
			StringTokenizer st1(topic, "/", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
			string s = st1[st1.count() - 1]; // Get last item.
			series.insert(std::pair<string, string>(topic, s));
		}
	}
	else {
		// handle.
		cerr << "Connection failed. Aborting subscribing.\n";
	}
}


// --- ON MESSAGE ---
void MtH::on_message(const struct mosquitto_message* message) {
	string topic = message->topic;	
	map<string, string>::iterator it = series.find(topic);
	if (it == series.end()) { 
		cerr << "Topic not found: " << topic << "\n";
		return; 
	}
	
	if (message->payloadlen < 1) {
		cerr << "No payload found. Returning...\n";
		return;
	}
	
	string payload = string((const char*) message->payload, message->payloadlen);
	size_t pos = payload.find(";");
	if (pos == string::npos || pos == 0) {
		// Invalid payload. Reject.
		cerr << "Invalid payload: " << payload << ". Reject.\n";
		return;
	}
	
	// Assemble the message to send to the InfluxDB instance. 
	// This message contains the uid and value from the MQTT payload, as well
	// as information from the listener's configuration.
	// Note: The timestamp is currently added by the InfluxDB, which is why it's
	// commented out here.
	// TODO: is a space (0x20) a valid UID?
	string uid = payload.substr(0, pos);
	string value = payload.substr(pos + 1);
	string influxMsg;	
	influxMsg = series[topic];
	influxMsg += ",location=" + uid;
	influxMsg += " value=" + value;
	//influxMsg += " " + to_string(static_cast<long int>(time(0)));
	
	// Send message.
	try {
		Net::HTTPRequest request(Net::HTTPRequest::HTTP_POST, "/write?db=" + influxDb, Net::HTTPMessage::HTTP_1_1);
		request.setContentLength(influxMsg.length());
		request.setContentType("application/x-www-form-urlencoded");
		influxClient->sendRequest(request) << influxMsg;
		
		Net::HTTPResponse response;
		influxClient->receiveResponse(response);
		/* Poco::Net::HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != Poco::Net::HTTPResponse::HTTP_OK && status != Poco::Net::HTTPResponse::HTTP_NO_CONTENT) {
			cerr << "Received InfluxDB error: " << response.getReason() << "\n";
		} */
	}
	catch (Exception& exc) {
		cout << "Exception caught while attempting to connect." << std::endl;
        cerr << exc.displayText() << std::endl;
        return;
	}
}


// --- ON SUBSCRIBE ---
void MtH::on_subscribe(int mid, int qos_count, const int* granted_qos) {
	// Report success, with details.
}
