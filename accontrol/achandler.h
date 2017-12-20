/*
	achandler.h - Header file for the ACHandler class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/10/29, Maya Posch <posch@synyx.de>
*/


#ifndef ACHANDLER_H
#define ACHANDLER_H

#include <iostream>
#include <vector>

using namespace std;

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::JSON;

#include "nodes.h"


class ACHandler: public HTTPRequestHandler { 
public: 
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
		// Process the request. Valid API calls:
		//
		// * GET /ac
		// -> Returns list of available AC units.
		// * GET /ac/<id>
		// -> Returns info on specified AC unit, or 404.
		// * POST /ac/<id>
		// -> Sets the target temperature for the specified AC unit.
		
		cout << "ACHandler: Request from " + request.clientAddress().toString() << "\n";
		
		URI uri(request.getURI());
		vector<string> parts;
		uri.getPathSegments(parts);
		
		response.setContentType("application/json"); // JSON mime-type
		response.setChunkedTransferEncoding(true); 
		
		// First path segment is 'ac'. If there's no second segment, return the
		// list of units. Otherwise check there's a valid ID and whether it's a 
		// POST or GET request.
		if (parts.size() == 1) {
			// Return list.
			ostream& ostr = response.send();
			ostr << "{ }";
		}
		else if (parts.size() == 2) {
			string id = parts[1];
			
			// Check in database. Return 404 if ID not found.
			NodeInfo info;
			if (!Nodes::getNodeInfo(id, info)) {
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				ostream& ostr = response.send();
				ostr << "{ \"error\": \"Node ID doesn't exist\" }";
				return;
			}
			
			// Check POST or GET.
			string method = request.getMethod();
			
			// Process request.
			if (method == HTTPRequest::HTTP_GET) {
				// Return info on ID.
				ostream& ostr = response.send();
				ostr << "{ \"id\": \"" << info.uid << "\",";
				ostr << "\"temperatureCurrent\": " << info.current << ",";
				ostr << "\"temperatureTarget\": " << info.target << "";
				ostr << "}";
			}
			else if (method == HTTPRequest::HTTP_POST) {
				// Validate and set provided data for ID.
				istream &i = request.stream();
				int len = request.getContentLength();
				char* buffer = new char[len];
				i.read(buffer, len);
				string content = string(buffer, len);
				delete[] buffer;
				
				Parser parser;
				Dynamic::Var result = parser.parse(content);
				Object::Ptr object = result.extract<Object::Ptr>();
				if (!object->has("temperatureTarget")) {
					response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
					ostream& ostr = response.send();
					ostr << "{ \"error\": \"Invalid request.\" }";
					return;
				}
				
				float newtemp = object->getValue<float>("temperatureTarget");
				
				// Validate temperature.
				if (newtemp <= 15 || newtemp >= 30) {
					response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
					ostream& ostr = response.send();
					ostr << "{ \"error\": \"Target temperature must be between 15 and 30 degrees.\" }";
					return;
				}
				
				info.target = newtemp;
				
				// Set new target temperature in the database.
				if (!Nodes::setTargetTemperature(info.uid, info.target)) {
					response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					ostream& ostr = response.send();
					ostr << "{ \"error\": \"Updating target temperature failed.\" }";
					return;
				}
				
				// Return info on ID.
				ostream& ostr = response.send();
				ostr << "{ \"id\": \"" << info.uid << "\",";
				ostr << "\"temperatureCurrent\": " << info.current << ",";
				ostr << "\"temperatureTarget\": " << info.target << "";
				ostr << "}";
			}
		}
		else {
			// Set 400 error.
			response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
			ostream& ostr = response.send();
			ostr << "{ \"error\": \"Invalid request.\" }";
		}
	}
};

#endif
