/*
	cchandler.h - Header file for the CCHandler class.
	
	Revision 0
	
	Notes:
			- 
			
	2022/07/30, Maya Posch
*/


#ifndef CCHANDLER_H
#define CCHANDLER_H

#include <iostream>
#include <vector>

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


class CCHandler: public HTTPRequestHandler { 
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
		
		std::cout << "CCHandler: Request from " + request.clientAddress().toString() << "\n";
		
		URI uri(request.getURI());
		std::vector<std::string> parts;
		uri.getPathSegments(parts);
		
		response.setContentType("application/json"); // JSON mime-type
		response.setChunkedTransferEncoding(true); 
		
		// First path segment is 'cc'. If there's no second segment, return the
		// list of units. Otherwise check there's a valid ID and whether it's a 
		// POST or GET request.
		if (parts.size() == 1) {
			// Return list.
			std::ostream& ostr = response.send();
			ostr << "{ \"nodes\": " << Nodes::nodesToJson() << 
					", \"unassigned\": " << Nodes::unassignedToJson() << " }";
		}
		else if (parts.size() == 2) {
			std::string id = parts[1];
			
			// Check in database. Return 404 if ID not found.
			NodeInfo info;
			if (!Nodes::getNodeInfo(id, info)) {
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				std::ostream& ostr = response.send();
				ostr << "{ \"error\": \"Node ID doesn't exist\" }";
				return;
			}
			
			// Check POST or GET.
			std::string method = request.getMethod();
			
			// Process request.
			if (method == HTTPRequest::HTTP_GET) {
				// Return info on ID.
				std::ostream& ostr = response.send();
				ostr << "{ \"id\": \"" << info.uid << "\",";
				ostr << "\"temperatureCurrent\": " << info.current << ",";
				ostr << "\"temperatureTarget\": " << info.target << "";
				ostr << "}";
			}
			else if (method == HTTPRequest::HTTP_POST) {
				// Validate and set provided data for ID.
				std::istream &i = request.stream();
				int len = request.getContentLength();
				char* buffer = new char[len];
				i.read(buffer, len);
				std::string content = std::string(buffer, len);
				delete[] buffer;
				
				Parser parser;
				Dynamic::Var result = parser.parse(content);
				Object::Ptr object = result.extract<Object::Ptr>();
				if (!object->has("temperatureTarget")) {
					response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
					std::ostream& ostr = response.send();
					ostr << "{ \"error\": \"Invalid request.\" }";
					return;
				}
				
				float newtemp = object->getValue<float>("temperatureTarget");
				
				// Validate temperature.
				if (newtemp <= 15 || newtemp >= 30) {
					response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
					std::ostream& ostr = response.send();
					ostr << "{ \"error\": \"Target temperature must be between 15 and 30 degrees.\" }";
					return;
				}
				
				info.target = newtemp;
				
				// Set new target temperature in the database.
				if (!Nodes::setTargetTemperature(info.uid, info.target)) {
					response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					std::ostream& ostr = response.send();
					ostr << "{ \"error\": \"Updating target temperature failed.\" }";
					return;
				}
				
				// Return info on ID.
				std::ostream& ostr = response.send();
				ostr << "{ \"id\": \"" << info.uid << "\",";
				ostr << "\"temperatureCurrent\": " << info.current << ",";
				ostr << "\"temperatureTarget\": " << info.target << "";
				ostr << "}";
			}
		}
		else if (parts.size() == 3) {
			if (parts[1] != "nodes") {
				// Set 400 error.
				response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
				std::ostream& ostr = response.send();
				ostr << "{ \"error\": \"Invalid request.\" }";
			}
			
			if (parts[2] == "assigned") {
				//Return list.
				std::ostream& ostr = response.send();
				ostr << "{ \"nodes\": " << Nodes::nodesToJson() << " }";
			}
			else if (parts[2] == "unassigned") {
				//Return list.
				std::ostream& ostr = response.send();
				ostr << "{ \"unassigned\": " << Nodes::unassignedToJson() << " }";
			}
			else if (parts[2] == "update") {
				// Check POST or GET.
				std::string method = request.getMethod();
			
				// Parse JSON, update local node information.
				if (method == HTTPRequest::HTTP_POST) {
					// Set 400 error.
					response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
					std::ostream& ostr = response.send();
					ostr << "{ \"error\": \"Invalid request.\" }";
				}
					
				// Validate and set provided data for ID.
				std::istream &i = request.stream();
				int len = request.getContentLength();
				char* buffer = new char[len];
				i.read(buffer, len);
				std::string content = std::string(buffer, len);
				delete[] buffer;
				
				Parser parser;
				Dynamic::Var result = parser.parse(content);
				Object::Ptr object = result.extract<Object::Ptr>();
				if (!object->has("uid")) {
					response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
					std::ostream& ostr = response.send();
					ostr << "{ \"error\": \"Invalid request.\" }";
					return;
				}
				
				// Get the values from the JSON object.
				NodeInfo node;
				node.uid = object->getValue<std::string>("uid");
				node.location = object->getValue<std::string>("location");
				
				// Modules section.
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
				Object::Ptr modobj = object->getObject("modules");
				bool thp 	= object->getValue<bool>("THP");
				bool co2 	= object->getValue<bool>("CO2");
				bool jura 	= object->getValue<bool>("Jura");
				bool jt 	= object->getValue<bool>("JuraTerm");
				bool motion = object->getValue<bool>("Motion");
				bool pwm 	= object->getValue<bool>("PWM");
				bool io		= object->getValue<bool>("IO");
				bool sw		= object->getValue<bool>("Switch");
				bool plant	= object->getValue<bool>("Plant");
				
				node.modules = 0;
				if (thp) 	{ node.modules |= 0x01; }
				if (co2) 	{ node.modules |= 0x02; }
				if (jura) 	{ node.modules |= 0x04; }
				if (jt) 	{ node.modules |= 0x08; }
				if (motion) { node.modules |= 0x10; }
				if (pwm) 	{ node.modules |= 0x20; }
				if (io) 	{ node.modules |= 0x40; }
				if (sw) 	{ node.modules |= 0x80; }
				if (plant) 	{ node.modules |= 0x100; }
				
				bool res = Nodes::updateNodeInfo(node.uid, node);
				
				// Validate temperature.
				if (res) {
					response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
					std::ostream& ostr = response.send();
					ostr << "{ \"error\": \"Failed to update node data.\" }";
					return;
				}
				
				// Return response.
				std::ostream& ostr = response.send();
				ostr << "{ \"error\": \"No error.\"";
				ostr << "}";
			}
			else {
				// Set 400 error.
				response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
				std::ostream& ostr = response.send();
				ostr << "{ \"error\": \"Invalid request.\" }";
			}
		}
		else {
			// Set 400 error.
			response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
			std::ostream& ostr = response.send();
			ostr << "{ \"error\": \"Invalid request.\" }";
		}
	}
};

#endif
