/*
	coffeenet.cpp - Implementation of the Coffeenet class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/10/29, Maya Posch <posch@synyx.de>
*/


#include "coffeenet.h"

#include <iostream>

using namespace std;

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/UUIDGenerator.h>

using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Net;


// Static initialisations.
string Coffeenet::eurekaHost;
string Coffeenet::eurekaPath;
bool Coffeenet::eurekaSecure;
string Coffeenet::authHost;
string Coffeenet::authPath;
bool Coffeenet::authSecure;
HTTPClientSession* Coffeenet::eurekaClient;
HTTPClientSession* Coffeenet::authClient;
Timer* Coffeenet::eurekaTimer;
Coffeenet* Coffeenet::selfRef = 0;
map<string, UserInfo> Coffeenet::userInfo;
queue<UserInfo*> Coffeenet::expirationQueue;
Timestamp Coffeenet::timestamp;
Mutex Coffeenet::userInfoMutex;
string Coffeenet::appId = "coffee";
string Coffeenet::appSecret = "secretKey";


// --- START ---
void Coffeenet::start(string eurekaServer, string eurekaSec, 
					string eurekaPath, string authServer, string authSec, string authPath) {
	Coffeenet::eurekaPath = eurekaPath;
	Coffeenet::eurekaHost = eurekaServer;
	Coffeenet::authHost = authServer;
	Coffeenet::authPath = authPath;
	if (eurekaSec == "true") { 
		cout << "Connecting to Eureka with HTTPS..." << std::endl;
		eurekaClient = new HTTPSClientSession(eurekaServer, 443);
		eurekaSecure = true; 
	} 
	else {
		cout << "Connecting to Eureka with HTTP..." << std::endl;
		eurekaClient = new HTTPClientSession(eurekaServer, 80);
		eurekaSecure = false; 
	}
	
	if (authSec == "true") { 
		cout << "Connecting to Auth with HTTPS..." << std::endl;
		authClient = new HTTPSClientSession(authServer, 443);
		authSecure = true; 
	} 
	else {
		cout << "Connecting to Auth with HTTP..." << std::endl;
		authClient = new HTTPClientSession(authServer, 80);
		authSecure = false; 
	}
	
	eurekaTimer = 0;
}


// --- STOP ---
void Coffeenet::stop() {
	// Deregister application.
	// DELETE request to 'apps/<AppId>/<instanceId>
	string path = eurekaPath + "apps/accontrol/10.0.15.44:8081";
	
	try {			
		HTTPRequest request(HTTPRequest::HTTP_DELETE, path, HTTPMessage::HTTP_1_1);
		eurekaClient->sendRequest(request);
		
		HTTPResponse response;
		istream &rs = eurekaClient->receiveResponse(response);
		HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != HTTPResponse::HTTP_OK) {
			cerr << "Received Eureka error: " << response.getReason() << "\n";
			// TODO: handle error.
			
			return;
		}
	}
	catch (Exception& exc) {
		cout << "Exception caught while attempting to connect." << endl;
		cerr << exc.displayText() << endl;
		return;
	}
	
	if (eurekaTimer) {
		eurekaTimer->stop();
		delete eurekaTimer;
	}
	
	if (selfRef) {
		delete selfRef;
	}
	
	delete eurekaClient;
	delete authClient;
}


bool Coffeenet::registerApp() {
	// JSON for registration:
	string regJSON = "{\
	\"instance\": {\
		\"instanceId\": \"10.0.15.44:8081\",\
		\"hostName\": \"10.0.15.44\",\
		\"app\": \"accontrol\",\
		\"vipAddress\": \"accontrol\",\
		\"secureVipAddress\": \"accontrol\",\
		\"ipAddr\": \"10.0.15.44\",\
		\"status\": \"UP\",\
		\"port\": {\"$\": \"8081\", \"@enabled\": \"true\"},\
		\"securePort\": {\"$\": \"8443\", \"@enabled\": \"false\"},\
		\"healthCheckUrl\": \"http://iot-central.synyx.coffee:8081/health\",\
		\"statusPageUrl\": \"http://iot-central.synyx.coffee:8081/info\",\
		\"homePageUrl\": \"http://iot-central.synyx.coffee:8081\",\
		\"dataCenterInfo\": {\
			\"@class\": \"com.netflix.appinfo.InstanceInfo$DefaultDataCenterInfo\", \
			\"name\": \"MyOwn\"\
		},\
	}";
	
	// Target path: POST to $Root/apps/appID
	string path = eurekaPath + "apps/accontrol";
	
	// Register app.
	try {			
		HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
		request.setContentType("application/json");
		eurekaClient->sendRequest(request);
		
		HTTPResponse response;
		istream &rs = eurekaClient->receiveResponse(response);
		HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != HTTPResponse::HTTP_NO_CONTENT) {
			cerr << "Received Eureka error: " << response.getReason() << "\n";
			cerr << "Aborting Eureka registration.\n";
			return false;
		}
	}
	catch (Exception& exc) {
		cout << "Exception caught while attempting to connect." << endl;
		cerr << exc.displayText() << endl;
		return false;
	}
	
	// Start heartbeat timer.
	eurekaTimer = new Timer(1000, 30 * 1000);	// Wait 1 second, 30 s interval.
	selfRef = new Coffeenet;
	TimerCallback<Coffeenet> eurekaCb(*selfRef, &Coffeenet::heartbeatCallback);
	eurekaTimer->start(eurekaCb);

	return true;
}


// --- AUTHENTICATE ---
// Authenticates this app.
bool Coffeenet::authenticate() {
	//
	
	return true;
}


// --- AUTHENTICATE USER ---
// Authenticates a user.
bool Coffeenet::authenticateUser(string token, UserInfo &info) {
	// Remove expired sessions.
	userInfoMutex.lock();
	map<string, UserInfo>::iterator it;
	while (expirationQueue.size() > 0 && 
				expirationQueue.front()->expired < timestamp.epochTime()) {
		it = userInfo.find(expirationQueue.front()->id);
		if (it != userInfo.end()) { userInfo.erase(it); }
		expirationQueue.pop();
	}
	
	it = userInfo.find(token);
	if (it == userInfo.end()) {
		return false; // User not found.
	}
	
	if (it->second.ip != info.ip) {
		// Wrong IP address. Possibly session being hijacked.
		return false;
	}
	
	// User found. Read out user info and return.
	info = it->second;	
	userInfoMutex.unlock();
	
	return true;
}


// --- AUTHENTICATE WITH TOKEN ---
// Authenticate a user using the provided authorisation code.
bool Coffeenet::authenticateWithCode(string code, UserInfo &info) {
	string path = authHost + authPath + "token?client_id=" + appId + "&client_secret=" + appSecret + "&grant_type=authorization_code&code=" + code + "&redirect_uri=http://iot-central.synyx.coffee:8081/login";
	
	string responseStr;
	try {			
		HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
		authClient->sendRequest(request);
		
		HTTPResponse response;
		istream &rs = authClient->receiveResponse(response);
		HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != HTTPResponse::HTTP_NO_CONTENT) {
			cerr << "Received OAuth error: " << response.getReason() << "\n";
			return false;
		}
		
		StreamCopier::copyToString(rs, responseStr);
	}
	catch (Exception& exc) {
		cout << "Exception caught while attempting to connect." << endl;
		cerr << exc.displayText() << endl;
		return false;
	}
	
	// The response should be JSON with the following format:
	// {
	//	  "access_token":"RsT5OjbzRn430zqMLgV3Ia",
	//	  "expires_in":3600
	//	}
	Parser parser;
	Dynamic::Var result = parser.parse(responseStr);
	Object::Ptr object = result.extract<Object::Ptr>();
	if (object->has("access_token")) {
		info.token = object->getValue<string>("access_token");
		info.expire = object->getValue<UInt32>("expires_in"); // in seconds.
	}
	else {
		// Handle error.
		cerr << "Authentication failed.\n";
		return false;
	}
	
	// Obtain info for this user from the OAuth server.
	path = authHost + authPath + "user?access_token=" + info.token;
	try {			
		HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
		//request.add("Authorization", "Bearer " + info.token);
		authClient->sendRequest(request);
		
		HTTPResponse response;
		istream &rs = authClient->receiveResponse(response);
		HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != HTTPResponse::HTTP_NO_CONTENT) {
			cerr << "Received OAuth error: " << response.getReason() << "\n";
			return false;
		}
		
		StreamCopier::copyToString(rs, responseStr);
	}
	catch (Exception& exc) {
		cout << "Exception caught while attempting to connect." << endl;
		cerr << exc.displayText() << endl;
		return false;
	}
	
	// The response should be JSON with the following format (regular user):
	// {
	//	  	"id":"${username}",
	//		"name":"${username}",
	//		"email":"${email}",
	//		"clientOnly":false,
	//		"principal": {
    //			"mail":"${email}",
    //			"authorities":[],
    //			"username":"${username}"
    //		}
	//	}
	result = parser.parse(responseStr);
	object = result.extract<Object::Ptr>();
	if (object->has("name")) {
		info.username = object->getValue<string>("name");
		info.email = object->getValue<string>("email");
	}
	else {
		// Handle error.
		cerr << "Obtaining user info failed.\n";
		return false;
	}
	
	// Generate new Coffeenet session ID, update local session info (user info).
	info.id = UUIDGenerator::defaultGenerator().createOne().toString();
	
	// Add current timestamp to the 'expired' member. This way we can see when
	// the session has expired and can be removed.
	info.expired = timestamp.epochTime() + info.expire; // in seconds.
	
	userInfoMutex.lock();
	std::pair<map<string, UserInfo>::iterator, bool> p;
	p = userInfo.insert(std::pair<string, UserInfo>(info.id, info));
	if (!p.second) {
		// Inserting new session failed. Cancel.
		cerr << "Failed to insert new session Possible UUID collision.\n";
		return false;
	}
	
	expirationQueue.push(&(p.first->second));
	userInfoMutex.unlock();
	
	return true;
}


// -- REMOVE USER ---
// Remove the specified user from local storage.
bool Coffeenet::removeUser(UserInfo &info) {
	userInfoMutex.lock();
	map<string, UserInfo>::iterator it;
	it = userInfo.find(info.id);
	if (it != userInfo.end()) {
		userInfo.erase(it);
	}
	
	userInfoMutex.unlock();
	return true;
}


// --- GET APP LIST ---
//
bool Coffeenet::getAppList(vector<AppInfo> &apps) {
	string path = eurekaPath + "apps";
	string responseStr;
	try {			
		HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
		request.add("Accept", "application/json"); // Request JSON.
		eurekaClient->sendRequest(request);
		
		HTTPResponse response;
		istream &rs = eurekaClient->receiveResponse(response);
		HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != HTTPResponse::HTTP_OK) {
			cerr << "Received Eureka error: " << response.getReason() << "\n";
			// TODO: handle error.
			
			return false;
		}
		
		StreamCopier::copyToString(rs, responseStr);
	}
	catch (Exception& exc) {
		cout << "Exception caught while attempting to connect." << endl;
		cerr << exc.displayText() << endl;
		return false;
	}
	
	// We should now have received a JSON object containing all of the apps.	
	Parser parser;
	Dynamic::Var result = parser.parse(responseStr);
	Object::Ptr object = result.extract<Object::Ptr>();
	if (object->has("applications")) {
		Object::Ptr appObj = object->getObject("applications");
		if (appObj->has("application")) {
			// Extract array and iterate through it, copying apps.
			Array::Ptr arr = appObj->getArray("application");
			size_t arrSize = arr->size();
			for (int i = 0; i < arrSize; ++i) {
				Object::Ptr app = arr->getObject(i);
				Poco::DynamicStruct ds = *app;
				
				// Check meta data whether to skip the app or not.
				// TODO: check authority of current user.
				//if (ds["instance"][0]["metadata"].contains("allowedAuthorities")) {
					try {
						if (ds["instance"][0]["metadata"]["allowedAuthorities"] 
													== "ROLE_COFFEENET-ADMIN") {
							continue;
						}
					}
					catch (NotFoundException& exc) {
						// Some apps don't have this property set. We can add these
						// too. Don't do anything here.
					}
				//}
				
				AppInfo info;
				info.name = Dynamic::Var::toString(ds["instance"][0]["vipAddress"]);
				info.url = Dynamic::Var::toString(ds["instance"][0]["homePageUrl"]);
				apps.push_back(info);
			}
		}
	}
	else {
		// Handle error.
		cerr << "Retrieving list of applications failed.\n";
		return false;
	}
	
	return true;
}


// --- GET APPS JSON ---
//
string Coffeenet::getAppsJson() {
	vector<AppInfo> apps;
	getAppList(apps);
	string json = "";
	UInt32 size = apps.size();
	for (UInt32 i = 0; i < size; ++i) {
		json += "{ \"name\": " + apps[i].name + ", ";
		json += "\"url\": " + apps[i].url + ",";
		json += "\"authorities\": [] },";
	}
	
	if (size > 0) {
		// Remove last comma.
		json.pop_back();
	}
	
	return json;
}


// --- HEART BEAT CALLBACK ---
// Called to send a notification to the Eureka server to tell we're still alive.
void Coffeenet::heartbeatCallback(Timer& timer) {
	// PUT request to 'apps/<AppId>/<instanceId>
	string path = eurekaPath + "apps/accontrol/10.0.15.44:8081";
	
	try {			
		HTTPRequest request(HTTPRequest::HTTP_PUT, path, HTTPMessage::HTTP_1_1);
		eurekaClient->sendRequest(request);
		
		HTTPResponse response;
		istream &rs = eurekaClient->receiveResponse(response);
		HTTPResponse::HTTPStatus status = response.getStatus();
		if (status != HTTPResponse::HTTP_OK) {
			cerr << "Received Eureka heartbeat error: " << response.getReason() << "\n";
			// TODO: handle heartbeat error. Re-register?
			
			return;
		}
	}
	catch (Exception& exc) {
		cout << "Exception caught while attempting to connect." << endl;
		cerr << exc.displayText() << endl;
		return;
	}
}
