/*
	datahandler.h - Header file for the BMaC Controller DataHandler class.
	
	Revision 0
	
	Notes:
			- 
			
	2022/07/30, Maya Posch
*/


#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <iostream>
#include <vector>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <Poco/File.h>

using namespace Poco::Net;
using namespace Poco::Data::Keywords;
using namespace Poco;

//#include "coffeenet.h"


class DataHandler: public HTTPRequestHandler { 
public: 
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
		// Process the request. A request for data (HTML, JS, etc.) is returned
		// with either the file & a 200 response, or a 404.
		//
		// TODO: OAuth2 authentication of client against authentication server.
		
		std::cout << "DataHandler: Request from " + request.clientAddress().toString() << std::endl;
		
		URI uri(request.getURI());
		std::string path = uri.getPath();
		/* if (path != "/") {
			// Return a 404.
			response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
			std::ostream& ostr = response.send();
			ostr << "File Not Found: " << path;
			return;
		} */
		
		URI::QueryParameters parts;
		parts = uri.getQueryParameters();
		if (parts.size() > 0 && parts[0].first == "uid") {
			// Check whether we have this UID in the database.
			// Set up SQLite database link.
			// TODO: use Session Pool for SQLite sessions.
			Data::SQLite::Connector::registerConnector();
			Data::Session* session = new Poco::Data::Session("SQLite", "nodes.db");
			
			Data::Statement select(*session);
			std::string filename;
			select << "SELECT file FROM firmware WHERE uid=?",
						into (filename),
						use (parts[0].second);
			
			size_t rows = select.execute();
			
			if (rows != 1) {
				// Return 404.
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				std::ostream& ostr = response.send();
				ostr << "File Not Found: " << parts[0].second;
				return;
			}
			
			// Return 200 OK with the firmware data, or a 404 if not found.
			std::string fileroot = "firmware/";
			File file(fileroot + filename);
			
			if (!file.exists() || file.isDirectory()) {
				// Return a 404.
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				std::ostream& ostr = response.send();
				ostr << "File Not Found.";
				return;
			}
			
			std::string mime = "application/octet-stream";
			try {
				response.sendFile(file.path(), mime);
			}
			catch (FileNotFoundException &e) {
				std::cout << "File not found exception triggered..." << std::endl;
				std::cerr << e.displayText() << std::endl;
				
				// Return a 404.
				response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
				std::ostream& ostr = response.send();
				ostr << "File Not Found.";
				return;
			}
			catch (OpenFileException &e) {
				std::cout << "Open file exception triggered..." << std::endl;
				std::cerr << e.displayText() << std::endl;
				
				// Return a 500.
				response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				std::ostream& ostr = response.send();
				ostr << "Internal Server Error. Couldn't open file.";
				return;
			}
		}
		/* else {
			// Return Bad Request
			response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
			response.send();
			return;
		} */
		
		// Validate user session.
		// Get the cookies for this domain from the user, check whether we have
		// a session token.
		/* NameValueCollection cookies;
		request.getCookies(cookies);
		bool authenticated = false;
		string token;
		if (cookies.has("CoffeeToken")) {
			token = cookies.get("CoffeeToken");
			authenticated = true; 
		}
		
		UserInfo info;
		info.ip = request.clientAddress(); */
		
		// Get the path and check for any endpoints to filter on.
		// FIXME: disable most endpoints for now until OAuth2 communication 
		// (in the Coffeenet class) can be fully debugged.
		/* HTTPCookie cookie;
		if (path == "/login") {
			// We're expecting to have been given an access token here.
			// Get the URI parameters and check.
			//URI uri(request.getURI());
			vector<std::pair<string, string> > parts; // QueryParameters
			//URI::QueryParameters parts;
			parts = uri.getQueryParameters();
			if (parts.size() > 0 && parts[0].first == "code") {
				// We should have a code to use with the OAuth server now.
				string authCode = parts[0].second;
				if (authCode.empty()) {
					cerr << "Authentication code was empty.\n";
					return;
				}
				
				// Use this code to authenticate the user with a token.
				if (!Coffeenet::authenticateWithCode(authCode, info)) {
					// Authentication failed. Abort.
					return;
				}
				
				// Set cookie on user with the new session ID.
				cookie.setHttpOnly(true);
				cookie.setName("CoffeeToken");
				cookie.setValue(info.id);
				cookie.setMaxAge(info.expire);
				response.addCookie(cookie);
				response.send();
			}
			else {
				// Return an error.
				response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
				response.send();
			}
			
			return;
		}
		else if (!authenticated && (!Coffeenet::authenticateUser(token, info))) {
			// Redirect user to the Coffeenet login page.
			response.setStatus(HTTPResponse::HTTP_TEMPORARY_REDIRECT);
			response.add("Location", Coffeenet::getAuthLoginURL());
			response.send();
			
			return;
		}
		else*/ //if (path == "/coffeenet/navigation") {
			// Return the session info for this user.
			/* string output = "[{ \"currentCoffeeNetUser\": {\
								\"username\": \"" + info.username + "\",\
								\"email\": \"" + info.email + "\",\
								\"avatar\": \"" + info.avatar + "\"\
								},\
							\"coffeeNetApps\": [" + Coffeenet::getAppsJson() + "],\
							\"profileApp\": {\
								\"name\": \"Profile\",\
								\"url\": \"https://profile.synyx.coffee\",\
								\"authorities\": []\
							},\
							\"logoutPath\": \"/logout\",\
							\"coffeeNetNavigationAppInformation\": {\
								\"groupId\": \"coffee.synyx\",\
								\"artifactId\": \"iot\",\
								\"version\": \"1.10.0\",\
								\"parentVersion\": \"0.26.0\",\
								\"parentArtifactId\": \"coffeenet-starter-parent\",\
								\"parentGroupId\": \"coffee.synyx\"\
							}\
						}\
					]";
			
			response.setContentType("application/json"); // JSON mime-type
			ostream& ostr = response.send();
			ostr << output;
			return; */
		/*}
		else if (path == "/logout") {
			// Remove local session data for this user, remove cookie and
			// redirect to the logout endpoint of the auth server.
			Coffeenet::removeUser(info);
			
			cookie.setName("CoffeeToken");
			cookie.setValue(info.id);
			cookie.setMaxAge(0);
			response.addCookie(cookie);
			
			// Redirect.
			response.setStatus(HTTPResponse::HTTP_TEMPORARY_REDIRECT);
			response.add("Location", Coffeenet::getLogoutURL());
			response.send();
			return;
		} */
		
		// No endpoint found, continue serving the requested file.
		std::string fileroot = "htdocs";
		if (path.empty() || path == "/") { path = "/index.html"; }
		
		File file(fileroot + path);
		
		std::cout << "DataHandler: Request for " << file.path() << std::endl;
		
		if (!file.exists() || file.isDirectory()) {
			// Return a 404.
			response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
			std::ostream& ostr = response.send();
			ostr << "File Not Found.";
			return;
		}
		
		// Determine file type.
		std::string::size_type idx = path.rfind('.');
		std::string ext = "";
		if (idx != std::string::npos) {
			ext = path.substr(idx + 1);
		}
		
		std::string mime = "text/plain";
		if (ext == "html") { mime = "text/html"; }
		if (ext == "css") { mime = "text/css"; }
		else if (ext == "js") { mime = "application/javascript"; }
		else if (ext == "zip") { mime = "application/zip"; }
		else if (ext == "json") { mime = "application/json"; }
		else if (ext == "png") { mime = "image/png"; }
		else if (ext == "jpeg" || ext == "jpg") { mime = "image/jpeg"; }
		else if (ext == "gif") { mime = "image/gif"; }
		else if (ext == "svg") { mime = "image/svg"; }
		
		try {
			response.sendFile(file.path(), mime);
		}
		catch (FileNotFoundException &e) {
			std::cout << "File not found exception triggered...\n";
			std::cerr << e.displayText() << std::endl;
			
			// Return a 404.
			response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
			std::ostream& ostr = response.send();
			ostr << "File Not Found.";
			return;
		}
		catch (OpenFileException &e) {
			std::cout << "Open file exception triggered...\n";
			std::cerr << e.displayText() << std::endl;
			
			// Return a 500.
			response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
			std::ostream& ostr = response.send();
			ostr << "Internal Server Error. Couldn't open file.";
			return;
		}
	}
};

#endif
