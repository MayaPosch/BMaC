/*
	httprequestfactory.h - Header for the ACControl's request handler factory.
	
	Revision 0
	
	Notes:
			- 
	
	2017/10/27, Maya Posch <posch@synyx.de>
*/


#pragma once
#ifndef HTTPREQUESTFACTORY_H
#define HTTPREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

using namespace Poco::Net;

#include "achandler.h"
#include "datahandler.h"


class RequestHandlerFactory: public HTTPRequestHandlerFactory { 
public:
	RequestHandlerFactory() {}
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) {
		if (request.getURI().compare(0, 4, "/ac/") == 0) { return new ACHandler(); }
		else { return new DataHandler(); }
	}
};

#endif
