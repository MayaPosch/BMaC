/*
	httprequestfactory.h - Header for the BMaC controller's request handler factory.
	
	Revision 0
	
	Notes:
			- 
	
	2022/07/30, Maya Posch
*/


#pragma once
#ifndef HTTPREQUESTFACTORY_H
#define HTTPREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

using namespace Poco::Net;

#include "achandler.h"
#include "cchandler.h"
#include "datahandler.h"


class RequestHandlerFactory: public HTTPRequestHandlerFactory { 
public:
	RequestHandlerFactory() {}
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) {
		if (request.getURI().compare(0, 4, "/ac/") == 0) { return new ACHandler(); }
		else if (request.getURI().compare(0, 4, "/cc/") == 0) { return new CCHandler(); }
		else { return new DataHandler(); }
	}
};

#endif
