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

#include <nymph.h>

#include <thread>

// FIXME: convert to C++11
Condition gCon;
Mutex gMutex;


void signal_handler(int signal) {
	gCon.signal();
}


// --- LOG FUNCTION ---
void logFunction(int level, string logStr) {
	cout << level << " - " << logStr << endl;
}


// --- GET MAC ---
// Request MAC address for a new node instance.
// Returns a string type with the MAC.
NymphMessage* getNewMac() {
	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new NymphString(nodes.getMAC()));
	return returnMsg;
}


// --- REGISTER UART CB ---
// Register a callback function for a UART.
// 
NymphMessage* registerUartCb(int session, NymphMessage* msg, void* data) {
	//
}


NymphMessage* writeSPI(int session, NymphMessage* msg, void* data) {
	//
}


NymphMessage* readSPI(int session, NymphMessage* msg, void* data) {
	//
}


NymphMessage* writeI2C(int session, NymphMessage* msg, void* data) {
	//
}


NymphMessage* readI2C(int session, NymphMessage* msg, void* data) {
	//
}


int main() {
	// For this simulation, we need to set up a simulated building structure, with rooms containing
	// everything from sensors to AC units to plants to coffee machines to people, to get realistic 
	// input for the nodes and thus the back-ends.
	
	// We assume at this point that the back-end servers have been started already and are present
	// on the local network.
	
	// First, read in the building configuration file.
	Config config;
	config.load("config.cfg");
	
	// Configure and start the NymphRPC server instance.
	// We need to provide methods for the following actions:
	// * Registering UART callback(s).
	// * SPI access.
	// * I2C access.
	vector<NymphTypes> parameters;
	parameters.push_back(NYMPH_STRING);
	NymphMethod registerUartFunction("registerUart", parameters, NYMPH_BOOL);
	registerUartFunction.setCallback(Building::registerUartCb);
	NymphRemoteClient::registerMethod("registerUart", registerUartFunction);
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4004.
	NymphRemoteClient::start(4004);
	
	// Next, we want to construct the building. We do this by creating a Building class instance.
	// We pass this the desired building configuration.
	Building building(config);
	
	// Loop until the SIGINT signal has been received.
	gMutex.lock();
	gCon.wait(gMutex);
	
	// Clean-up
	NymphRemoteClient::shutdown();
	
	// Wait before exiting, giving threads time to exit.
	Thread::sleep(2000); // 2 seconds.
	
	return 0;
}
