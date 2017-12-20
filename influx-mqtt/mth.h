/*
	mth.h - Header file for the MQTT to HTTP class.
	
	Revision 0
	
	Notes:
			- Declares a class for converting from MQTT to InfluxDB HTTP requests.
			
	2017/02/09, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef MTH_H
#define MTH_H

#include <mosquittopp.h>

#include <string>
#include <map>

using namespace std;

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>


class MtH : public mosqpp::mosquittopp {
	Poco::Net::HTTPClientSession* influxClient;
	string topics;
	string influxDb;
	bool secure;
	map<string, string> series;
	
public:
	MtH(string clientId, string host, int port, string topics, 
					string influxHost, int influxPort, string influxDb, string influx_sec);
	~MtH();
	
	void on_connect(int rc);
	void on_message(const struct mosquitto_message* message);
	void on_subscribe(int mid, int qos_count, const int* granted_qos);
};

#endif
