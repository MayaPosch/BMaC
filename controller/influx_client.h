/*
	influx_client.h - Influx client for the BMaC Controller project.
	
	Features:
			- Writing data to a remote InfluxDB instance.
			- Reading specific data from an InfluxDB instance.
	
	Notes:
			- Uses the v1 API & InfluxQL currently. Using v2 API & Flux is WIP.
			
	2022/09/08, Maya Posch
*/


#ifndef INFLUX_CLIENT_H
#define INFLUX_CLIENT_H


#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>


class InfluxClient {
	static bool initialized;
	static Poco::Net::HTTPClientSession* influxClient;
	static std::string influxDb;
	static bool secure;
	
public:
	static bool init(std::string host, int port, std::string database, std::string secure);
	static bool writeQuery(std::string query);
};


#endif
