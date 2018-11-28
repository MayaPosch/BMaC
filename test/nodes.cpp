


#include "nodes.h"


// Static initialisations.
std::map<std::string, Node*> nodes;
std::queue<std::string> macs;


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


std::string Nodes::readUart(std::string mac) {
	Node* node = getNode(mac);
	if (!node) { return std::string(); }
	
	return node->readUart();
}


bool Nodes::writeSPI(std::string mac, std::string bytes) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	node->writeSPI(bytes);
	
	return true;
}


std::string Nodes::readSPI(std::string mac) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	return node->readSPI();
}


bool Nodes::writeI2C(std::string mac, std::string bytes) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	node->writeI2C(bytes);
	
	return true;
}


std::string Nodes::readI2C(std::string mac) {
	Node* node = getNode(mac);
	if (!node) { return false; }
	
	return node->readI2C();
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
