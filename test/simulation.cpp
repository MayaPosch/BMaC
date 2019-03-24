/*
	simulation.cpp - Main file for the BMaC simulation and validation framework.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2018/09/18, Maya Posch
*/


#include "config.h"
#include "building.h"
#include "nodes.h"

#include <nymph/nymph.h>

#include <thread>
#include <condition_variable>
#include <mutex>

// FIXME: convert to C++11
std::condition_variable gCon;
std::mutex gMutex;
bool gPredicate = false;


void signal_handler(int signal) {
	gPredicate = true;
	gCon.notify_one();
}


// --- LOG FUNCTION ---
void logFunction(int level, std::string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


// --- GET MAC ---
// Request MAC address for a new node instance.
// Returns a string type with the MAC.
NymphMessage* getNewMac(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get new MAC and register it with this session ID.
	std::string mac = Nodes::getMAC();
	Nodes::registerSession(mac, session);
	
	returnMsg->setResultValue(new NymphString(mac));
	return returnMsg;
}


// --- REGISTER UART CB ---
// Register a callback function for a UART.
// Requires the name of the callback on the client, returns a boolean result.
/* NymphMessage* registerUartCb(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the MAC address & callback name, then call the function on the associated Node instance.
	std::string mac = ((NymphString*) msg->parameters[0])->getValue();
	std::string cb = ((NymphString*) msg->parameters[1])->getValue();
	returnMsg->setResultValue(new NymphBoolean(Nodes::registerUartCb(mac, cb)));
	return returnMsg;
} */


// --- WRITE UART ---
// Writes data to the UART.
NymphMessage* writeUart(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the MAC address, then call the function on the associated Node instance.
	std::string mac = ((NymphString*) msg->parameters()[0])->getValue();
	std::string bytes = ((NymphString*) msg->parameters()[1])->getValue();
	returnMsg->setResultValue(new NymphBoolean(Nodes::writeUart(mac, bytes)));
	return returnMsg;
}


// --- SEND UART ---
// Send to the UART.
/* void sendUart(std::string data) { //(int session, NymphMessage* msg, void* data) {
	vector<NymphType*> values;
	values.push_back(new NymphString(data));
	string result;
	NymphBoolean* world = 0;
	if (!NymphRemoteClient::callCallback(session, "serialRxCallback", values, result)) {
		// std::cerr << "Calling callback failed: " << result << endl;
		// TODO: Report error.
	} */
	
	/* NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the MAC address, then call the function on the associated Node instance.
	std::string mac = ((NymphString*) msg->parameters()[0])->getValue();
	returnMsg->setResultValue(new NymphString(Nodes::readUart(mac)));
	return returnMsg; */
//}


// --- WRITE SPI ---
NymphMessage* writeSPI(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the MAC address, then call the function on the associated Node instance.
	std::string mac = ((NymphString*) msg->parameters()[0])->getValue();
	std::string bytes = ((NymphString*) msg->parameters()[1])->getValue();
	returnMsg->setResultValue(new NymphBoolean(Nodes::writeSPI(mac, bytes)));
	return returnMsg;
}


// --- READ SPI ---
NymphMessage* readSPI(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the MAC address, then call the function on the associated Node instance.
	std::string mac = ((NymphString*) msg->parameters()[0])->getValue();
	returnMsg->setResultValue(new NymphString(Nodes::readSPI(mac)));
	return returnMsg;
}


// --- WRITE I2C ---
NymphMessage* writeI2C(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the MAC address, then call the function on the associated Node instance.
	std::string mac = ((NymphString*) msg->parameters()[0])->getValue();
	int i2cAddress = ((NymphSint32*) msg->parameters()[1])->getValue();
	std::string bytes = ((NymphString*) msg->parameters()[2])->getValue();
	returnMsg->setResultValue(new NymphBoolean(Nodes::writeI2C(mac, i2cAddress, bytes)));
	return returnMsg;
}


// --- READ I2C ---
NymphMessage* readI2C(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the MAC address, then call the function on the associated Node instance.
	std::string mac = ((NymphString*) msg->parameters()[0])->getValue();
	int i2cAddress = ((NymphSint32*) msg->parameters()[1])->getValue();
	int length = ((NymphSint32*) msg->parameters()[2])->getValue();
	returnMsg->setResultValue(new NymphString(Nodes::readI2C(mac, i2cAddress, length)));
	return returnMsg;
}


int main() {
	// For this simulation, we need to set up a simulated building structure, with rooms containing
	// everything from sensors to AC units to plants to coffee machines to people, to get realistic 
	// input for the nodes and thus the back-ends.
	
	// We assume at this point that the back-end servers have been started already and are present
	// on the local network.
	
	// First, read in the building configuration file.
	Config config;
	if (!config.load("config.ini")) {
		std::cerr << "Couldn't load configuration file 'config.ini'" << std::endl;
		return 1;
	}
	
	
	// Initialise Nymph the server instance.
	std::cout << "Initialising server..." << std::endl;
	long timeout = 5000; // 5 seconds.
	NymphRemoteClient::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Configure and start the NymphRPC server instance.
	// We need to provide methods for the following actions:
	// * Get the network MAC.
	// * Registering UART callback(s).
	// * SPI access.
	// * I2C access.
	// * UART access.
	std::vector<NymphTypes> parameters;
	NymphMethod getNewMacFunction("getNewMac", parameters, NYMPH_STRING);
	getNewMacFunction.setCallback(getNewMac);
	NymphRemoteClient::registerMethod("getNewMac", getNewMacFunction);
	
	parameters.push_back(NYMPH_STRING);
	NymphMethod serialRxCallback("serialRxCallback", parameters, NYMPH_NULL);
	serialRxCallback.enableCallback();
	NymphRemoteClient::registerCallback("serialRxCallback", serialRxCallback);
	
	// bool registerUartCb(string MAC, string callbackName)
	/* parameters.push_back(NYMPH_STRING); // MAC
	parameters.push_back(NYMPH_STRING);	// Callback name.
	NymphMethod registerUartCbFunction("registerUartCb", parameters, NYMPH_BOOL);
	registerUartCbFunction.setCallback(Building::registerUartCb);
	NymphRemoteClient::registerMethod("registerUartCb", registerUartCbFunction); */
	
	// string readUart(string MAC)
	/* NymphMethod readUartFunction("readUart", parameters, NYMPH_STRING);
	readUartFunction.setCallback(readUart);
	NymphRemoteClient::registerMethod("readUart", readUartFunction); */
	
	// string readI2C(string MAC, int i2cAddress, int length)
	parameters.push_back(NYMPH_SINT32);
	parameters.push_back(NYMPH_SINT32);
	NymphMethod readI2CFunction("readI2C", parameters, NYMPH_STRING);
	readI2CFunction.setCallback(readI2C);
	NymphRemoteClient::registerMethod("readI2C", readI2CFunction);
	
	// bool writeUart(string MAC, string bytes)
	parameters.clear();
	parameters.push_back(NYMPH_STRING);
	parameters.push_back(NYMPH_STRING);
	NymphMethod writeUartFunction("writeUart", parameters, NYMPH_BOOL);
	writeUartFunction.setCallback(writeUart);
	NymphRemoteClient::registerMethod("writeUart", writeUartFunction);
	
	// bool writeSPI(string MAC, string bytes)
	NymphMethod writeSPIFunction("writeSPI", parameters, NYMPH_BOOL);
	writeSPIFunction.setCallback(writeSPI);
	NymphRemoteClient::registerMethod("writeSPI", writeSPIFunction);
	
	// bool writeI2C(string MAC, int i2cAddress, string bytes)
	parameters.clear();
	parameters.push_back(NYMPH_STRING);
	parameters.push_back(NYMPH_SINT32);
	parameters.push_back(NYMPH_SINT32);
	NymphMethod writeI2CFunction("writeI2C", parameters, NYMPH_BOOL);
	writeI2CFunction.setCallback(writeI2C);
	NymphRemoteClient::registerMethod("writeI2C", writeI2CFunction);
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4004.
	NymphRemoteClient::start(4004);
	
	// Next, we want to construct the building. We do this by creating a Building class instance.
	// We pass this the desired building configuration.
	Building building(config);
	
	// Loop until the SIGINT signal has been received.
	std::unique_lock<std::mutex> lock(gMutex);
	while (!gPredicate) {
		gCon.wait(lock);
	}
	
	// Clean-up
	NymphRemoteClient::shutdown();
	
	// Wait before exiting, giving threads time to exit.
	Poco::Thread::sleep(2000); // 2 seconds.
	
	return 0;
}
