/*
	listener.h - Header file for BMaC C&C MQTT listener.
	
	Revision 0
	
	Notes:
			- 
			
	2017/02/09, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef LISTENER_H
#define LISTENER_H

#include <mosquittopp.h>

#include <string>

using namespace std;


#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>

using namespace Poco;


class Listener : public mosqpp::mosquittopp {
	Data::Session* session;
	string defaultFirmware;
	
public:
	Listener(string clientId, string host, int port, string defaultFirmware);
	~Listener();
	
	void on_connect(int rc);
	void on_message(const struct mosquitto_message* message);
	void on_subscribe(int mid, int qos_count, const int* granted_qos);
};

#endif
