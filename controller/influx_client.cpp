/*
	influx_client.h - Influx client for the BMaC Controller project.
	
	Features:
			- Writing data to a remote InfluxDB instance.
			- Reading specific data from an InfluxDB instance.
	
	Notes:
			- Uses the v1 API & InfluxQL currently. Using v2 API & Flux is WIP.
			
	2022/09/08, Maya Posch
*/


#include "influx_client.h"


#include <iostream>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

// Static initialisations.
bool InfluxClient::initialized = false;
Poco::Net::HTTPClientSession* InfluxClient::influxClient;
std::string InfluxClient::influxDb;
bool InfluxClient::secure;


// --- INIT ---
bool InfluxClient::init(std::string host, int port, std::string database, std::string secure) {
	// Assign parameters.
	influxDb = database;
	if (secure == "true") { 
		std::cout << "Connecting with HTTPS..." << std::endl;
		influxClient = new Poco::Net::HTTPSClientSession(host, port);
		secure = true; 
	} 
	else {
		std::cout << "Connecting with HTTP..." << std::endl;
		influxClient = new Poco::Net::HTTPClientSession(host, port);
		secure = false; 
	}
	
	initialized = true;
	
	return true;
}


// --- WRITE QUERY ---
// Writes a data sequence into to the database using the previously configured database target.
bool InfluxClient::writeQuery(std::string query) {
	if (!initialized) { return false; }
	
	try {
		Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/write?db=" + influxDb, Poco::Net::HTTPMessage::HTTP_1_1);
		request.setContentLength(query.length());
		request.setContentType("application/x-www-form-urlencoded");
		influxClient->sendRequest(request) << query;
		
		Poco::Net::HTTPResponse response;
		influxClient->receiveResponse(response);
		Poco::Net::HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != Poco::Net::HTTPResponse::HTTP_OK && status != Poco::Net::HTTPResponse::HTTP_NO_CONTENT) {
			std::cerr << "Received InfluxDB error: " << response.getReason() << "\n";
		}
	}
	catch (Poco::Exception& exc) {
		std::cout << "Exception caught while attempting to connect." << std::endl;
		std::cerr << exc.displayText() << std::endl;
		return false;
	}
	catch (...) {
		std::cerr << "Caught unknown exception in InfluxClient::writeQuery." << std::endl;
		return false;
	}
	
	return true;
}
