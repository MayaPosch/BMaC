/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServer
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/** @defgroup   httpserver HTTP server
 *  @brief      Provides powerful HTTP/S + Websocket server
 *  @ingroup    tcpserver
 *  @{
 */

#ifndef _SMING_CORE_HTTPSERVER_H_
#define _SMING_CORE_HTTPSERVER_H_

//#include "TcpServer.h"
#include "../wiring/WString.h"
#include "../wiring/WHashMap.h"
#include "../Delegate.h"
#include "Http/HttpResponse.h"
/*#include "Http/HttpRequest.h"
#include "Http/HttpResource.h"
#include "Http/HttpServerConnection.h"
#include "Http/HttpBodyParser.h" */
//#include "Http/HttpResourceTree.h"

class HttpResource;
class HttpRequest;
class HttpResponse;

/* class HttpPathDelegate {
	//
}; */


typedef Delegate<void(HttpRequest& request, HttpResponse& response)> HttpPathDelegate;

class HttpResourceTree {
public:
	void setDefault(const HttpPathDelegate& callback) {
		//
	}
};

class HttpBodyParserDelegate {
	//
};

class HttpServerConnection {
	//
};
class HttpResourceDelegate {
	//
};

class TemplateVariables : public HashMap<String, String> {
};

class TemplateFileStream {
	String path;
	TemplateVariables var;
	
public:
	TemplateFileStream(String p) { path = p; }
	TemplateVariables& variables() { return var; }
};

class HttpResource {
	//
};

class HttpRequest {
	//
};

/* class HttpResponse {
	//
	
public:
	void sendTemplate(TemplateFileStream* fs) { }
}; */

typedef struct {
	int maxActiveConnections = 10;  // << the maximum number of concurrent requests..
	int keepAliveSeconds = 0;		// << the default seconds to keep the connection alive before closing it
	int minHeapSize = -1;			// << defines the min heap size that is required to accept connection.
									//  -1 - means use server default
	bool useDefaultBodyParsers = 1; // << if the default body parsers,  as form-url-encoded, should be used
#ifdef ENABLE_SSL
	int sslSessionCacheSize =
		10; // << number of SSL session ids to cache. Setting this to 0 will disable SSL session resumption.
#endif
} HttpServerSettings;

class HttpServer //: public TcpServer
{
	friend class HttpServerConnection;

public:
	HttpResourceTree paths;
	
	HttpServer();
	HttpServer(const HttpServerSettings& settings);
	virtual ~HttpServer();
	
	// Stubs.
	void listen(int port) { }
	void shutdown() { }

	/**
	 * @brief Allows changing the server configuration
	 */
	void configure(const HttpServerSettings& settings);

	/**
	 * @briefs Allows content-type specific parsing of the body based on content-type.
	 *
	 * @param const String& contentType. Can be full content-type like 'application/json', or 'application/*'  or '*'.
	 * 						If there is exact match for the content-type wildcard content-types will not be used.
	 * 						There can be only one catch-all '*' body parser and that will be the last registered
	 *
	 * @param  HttpBodyParserDelegate parser
	 */
	void setBodyParser(const String& contentType, HttpBodyParserDelegate parser);

	/**
	 * @param String path URL path.
	 * @note Path should start with slash. Trailing slashes will be removed.
	 * @param HttpPathDelegate callback - the callback that will handle this path
	 */
	void addPath(String path, const HttpPathDelegate& callback);
	void addPath(const String& path, const HttpResourceDelegate& onRequestComplete);
	void addPath(const String& path, HttpResource* resource);

	void setDefaultHandler(const HttpPathDelegate& callback);
	void setDefaultHandler(void (&)(HttpRequest&, HttpResponse&)) { }
	void setDefaultResource(HttpResource* resource);

protected:
	//virtual TcpConnection* createClient(tcp_pcb* clientTcp);

protected:
#ifdef ENABLE_SSL
	int minHeapSize = 16384;
#endif

private:
	HttpServerSettings settings;
	//ResourceTree resourceTree;
	//BodyParsers bodyParsers;
};

/** @} */
#endif /* _SMING_CORE_HTTPSERVER_H_ */
