/*
	coffeenet.h - Header file for the Coffeenet class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/10/29, Maya Posch <posch@synyx.de>
*/

#ifndef COFFEENET_H
#define COFFEENET_H


#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Timestamp.h>
#include <Poco/Timer.h>
#include <Poco/Mutex.h>

using namespace Poco;
using namespace Poco::Net;

#include <string>
#include <vector>
#include <map>
#include <queue>

using namespace std;


struct AppInfo {
	string name;
	string url;
};


struct UserInfo {
	string id;	// randomly generated ID, also stored in user cookie.
	SocketAddress ip;
	string token;	// access token.
	UInt32 expire;	// TTL for this session.
	time_t expired;	// when this session has expired (UNIX timestamp in us).
	string username;
	string email;
	string avatar;
};


class Coffeenet {
	static string eurekaHost;
	static string eurekaPath;
	static bool eurekaSecure;
	static string authHost;
	static string authPath;
	static bool authSecure;
	static HTTPClientSession* eurekaClient;
	static HTTPClientSession* authClient;
	static Timer* eurekaTimer;
	static Coffeenet* selfRef;
	static map<string, UserInfo> userInfo;
	static string appId;
	static string appSecret;
	static queue<UserInfo*> expirationQueue;
	static Timestamp timestamp;
	static Mutex userInfoMutex;
	
public:
	static void start(string eurekaServer, string eurekaSec, string eurekaPath, string authServer, string authSec, string authPath);
	static void stop();
	static bool registerApp();
	static bool authenticate();
	static bool authenticateWithCode(string code, UserInfo &info);
	static bool authenticateUser(string token, UserInfo &info);
	static bool removeUser(UserInfo &info);
	static bool getAppList(vector<AppInfo> &apps);
	static string getAppsJson();
	static string getAuthLoginURL() { 
		// TODO: add a unique 'state' reference.
		return "http://" + authHost + authPath + "authorize?client_id=iot&redirect_uri=http://iot-central.synyx.coffee:8081/login&response_type=code&state=ii76r43";
	}
	static string getLogoutURL() {
		return "http://" + authHost + "/logout?redirect=http://iot-central.synyx.coffee:8081";
	}
	
	// Callbacks
	void heartbeatCallback(Timer& timer);
};

#endif
