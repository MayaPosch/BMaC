/*
	httprequestfactory.h - Header for the C&C Server's request handler factory.
	
	Revision 0
	
	Notes:
			- 
	
	2017/12/21, Maya Posch
*/


#pragma once
#ifndef HTTPREQUESTFACTORY_H
#define HTTPREQUESTFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

using namespace Poco::Net;

#include "datahandler.h"


class RequestHandlerFactory: public HTTPRequestHandlerFactory { 
public:
	RequestHandlerFactory() {}
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) {
		return new DataHandler();
	}
};

#endif
