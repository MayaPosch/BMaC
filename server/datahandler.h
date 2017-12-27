/*
	datahandler.h - Header file for the DataHandler class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/12/22, Maya Posch
*/


#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <iostream>
#include <vector>

using namespace std;

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/File.h>

#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>

using namespace Poco::Data::Keywords;

using namespace Poco::Net;
using namespace Poco;


class DataHandler: public HTTPRequestHandler { 
public: 
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
		// We only handle requests for firmware images here, using a provided UID.
		// Return a 200 OK for success, 404 otherwise.
		
		cout << "DataHandler: Request from " + request.clientAddress().toString() << endl;
		
		URI uri(request.getURI());
		string path = uri.getPath();
		if (path != "/") {
			// Return a 404.
			response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
			ostream& ostr = response.send();
			ostr << "File Not Found: " << path;
			return;
		}
		
		URI::QueryParameters parts;
		parts = uri.getQueryParameters();
		if (parts.size() > 0 && parts[0].first == "uid") {
			// Check whether we have this UID in the database.
			// Set up SQLite database link.
			// TODO: use Session Pool for SQLite sessions.
			Data::SQLite::Connector::registerConnector();
			Data::Session* session = new Poco::Data::Session("SQLite", "nodes.db");
			
			Data::Statement select(*session);
			string filename;
			select << "SELECT file FROM firmware WHERE uid=?",
						into (filename),
						use (parts[0].second);
			
			size_t rows = select.execute();
			
			if (rows != 1) {
				// Return 404.
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				ostream& ostr = response.send();
				ostr << "File Not Found: " << parts[0].second;
				return;
			}
			
			// Return 200 OK with the firmware data, or a 404 if not found.
			string fileroot = "firmware/";
			File file(fileroot + filename);
			
			if (!file.exists() || file.isDirectory()) {
				// Return a 404.
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				ostream& ostr = response.send();
				ostr << "File Not Found.";
				return;
			}
			
			string mime = "application/octet-stream";
			try {
				response.sendFile(file.path(), mime);
			}
			catch (FileNotFoundException &e) {
				cout << "File not found exception triggered..." << endl;
				cerr << e.displayText() << endl;
				
				// Return a 404.
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				ostream& ostr = response.send();
				ostr << "File Not Found.";
				return;
			}
			catch (OpenFileException &e) {
				cout << "Open file exception triggered..." << endl;
				cerr << e.displayText() << endl;
				
				// Return a 500.
				response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				ostream& ostr = response.send();
				ostr << "Internal Server Error. Couldn't open file.";
				return;
			}
		}
		else {
			// Return Bad Request
			response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
			response.send();
			return;
		}
	}
};

#endif