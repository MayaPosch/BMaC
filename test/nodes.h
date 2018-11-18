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
	static std::map<std::string, Node*> nodes;
	static std::queue<std::string> macs;
	
public:
	static bool addNode(std::string mac, Node* node);
	static bool removeNode(std::string mac);
	static bool removeNode(Node* node);
	static void addMAC(std::string mac);
	static std::string getMAC();
};

#endif