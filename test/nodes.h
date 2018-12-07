/*
	nodes.h - Nodes static class.
	
	Revision 0
	
	Notes:
			- 
		
	2018/09/18, Maya Posch
*/


#ifndef NODES_H
#define NODES_H


#include <map>
#include <string>
#include <queue>


class Node;


class Nodes {
	static Node* getNode(std::string mac);
	
	static std::map<std::string, Node*> nodes;
	static std::queue<std::string> macs;
	
public:
	static bool addNode(std::string mac, Node* node);
	static bool removeNode(std::string mac);
	static bool removeNode(Node* node);
	//static bool registerUartCb(std::string mac, std::string cb);
	static bool writeUart(std::string mac, std::string bytes);
	static std::string readUart(std::string mac);
	static bool writeSPI(std::string mac, std::string bytes);
	static std::string readSPI(std::string mac);
	static bool writeI2C(std::string mac, int i2cAddress, std::string bytes);
	static std::string readI2C(std::string mac, int i2cAddress, int length);
	static void addMAC(std::string mac);
	static std::string getMAC();
};

#endif