/*
	nodes.cpp - Nodes static class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#include "nodes.h"
#include "node.h"
#include <nymph/nymph.h>

#include <iostream>

using namespace std;


// Static initialisations.
std::map<std::string, Node*> Nodes::nodes;
std::queue<std::string> Nodes::macs;
std::map<std::string, int> Nodes::sessions;
std::vector<Poco::ProcessHandle> Nodes::node_processes;


// --- GET NODE ---
Node* Nodes::getNode(std::string mac) {
	std::map<std::string, Node*>::iterator it;
	it = nodes.find(mac);
	if (it == nodes.end()) { return 0; }
	
	return it->second;
}


bool Nodes::addNode(std::string mac, Node* node) {
	std::pair<std::map<std::string, Node*>::iterator, bool> ret;
	ret = nodes.insert(std::pair<std::string, Node*>(mac, node));
	if (ret.second) { macs.push(mac); }
	return ret.second;
}


bool Nodes::removeNode(std::string mac) {
	std::map<std::string, Node*>::iterator it;
	it = nodes.find(mac);
	if (it == nodes.end()) { return false; }	
	nodes.erase(it);
	return true;
}


bool Nodes::removeNode(Node* node) {
	//
	
	return true;
}


void Nodes::registerSession(std::string mac, int session) {
	sessions.insert(std::pair<std::string, int>(mac, session));
}


/* bool Nodes::registerUartCb(std::string mac, std::string cb) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	node->registerUartCb(cb);
	
	return true;
} */


bool Nodes::writeUart(std::string mac, std::string bytes) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	node->writeUart(bytes);
	
	return true;
}


bool Nodes::sendUart(std::string mac, std::string bytes) {
	// Get the session ID, then call the remote callback function.
	std::map<std::string, int>::iterator it;
	it = sessions.find(mac);
	if (it == sessions.end()) { return false; }
	
	vector<NymphType*> values;
	values.push_back(new NymphString(bytes));
	string result;
	NymphBoolean* world = 0;
	if (!NymphRemoteClient::callCallback(it->second, "serialRxCallback", values, result)) {
		// std::cerr << "Calling callback failed: " << result << endl;
		// TODO: Report error.
	}
	
	return true;
}


/* std::string Nodes::readUart(std::string mac) {
	Node* node = getNode(mac);
	if (!node) { return std::string(); }
	
	return node->readUart();
} */


bool Nodes::writeSPI(std::string mac, std::string bytes) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	node->writeSPI(bytes);
	
	return true;
}


std::string Nodes::readSPI(std::string mac) {
	Node* node = getNode(mac);
	if (!node) { return std::string(); }
	
	return node->readSPI();
}


bool Nodes::writeI2C(std::string mac, int i2cAddress, std::string bytes) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	node->writeI2C(i2cAddress, bytes);
	
	return true;
}


std::string Nodes::readI2C(std::string mac, int i2cAddress, int length) {
	Node* node = getNode(mac);
	if (!node) { return std::string(); }
	
	return node->readI2C(i2cAddress, length);
}


void Nodes::addMAC(std::string mac) {
	macs.push(mac);
}


std::string Nodes::getMAC() {
	 if (macs.empty()) { return std::string(); }
	 
	 std::string val = macs.front();
	 macs.pop();
	 return val;
 }
 
 
void Nodes::addProcess(Poco::ProcessHandle pid) {
	// Debug.
	std::cout << "Nodes: Adding process handle..." << std::endl;
	
	node_processes.push_back(pid);
}


void Nodes::endProcesses() {
	// Debug.
	std::cout << "Nodes: Ending processes..." << std::endl;
	
	for (Poco::ProcessHandle pid : node_processes) {
		// Debug.
		std::cout << "Nodes: Ending process handle..." << std::endl;
	
		// TODO: implement RPC method to gracefully shut down the node processes instead of
		// simply killing them.
		Poco::Process::kill(pid);
	}
}
