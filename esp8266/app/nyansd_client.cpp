/*
	nyansd_client.cpp - Implementation file for the NyanSD service discovery client library.
	
	Notes:
			- Arduino-specific version of the full-fat version.
			
	2020/04/23, Maya Posch
	2022/08/09, Maya Posch - Adapted for Arduino.
*/


// Uncomment or define DEBUG to enable debug output.
//#define DEBUG 1


#include "nyansd_client.h"

#include <cstring>

//#include <UdpConnection.h>

//#include <iostream>
//#include <map>

#include <cstdlib>

#include <lwip/pbuf.h>
#include <lwip/ip.h>
#include <lwip/udp.h>

/* #include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/NetException.h>
#include <Poco/Exception.h> */


// Static variables.
/* std::vector<NYSD_service> NyanSD::services;
std::mutex NyanSD::servicesMutex;
std::atomic<bool> NyanSD::running{false};
std::thread NyanSD::handler;
ByteBauble NyanSD::bb; */

ListNode* NyanSD_client::response = 0;
ListNode* NyanSD_client::last = 0;
int NyanSD_client::count = 0;
bool NyanSD_client::received = false;
uint16_t NyanSD_client::port = 0;
UdpConnection NyanSD_client::udpsocket(udp_receive_callback);


struct ResponseStruct {
	char* data;
	uint32_t length;
};


// --- UDP RECEIVE CALLBACK --
// void arg					- User argument - udp_recv `arg` parameter 
// struct udp_pcb* upcb 	- Receiving Protocol Control Block
// struct pbuf* p			- Pointer to Datagram
// const ip_addr_t* addr 	- Address of sender 
// uint16_t port			- Sender port
/* void NyanSD_client::udp_receive_callback(void* arg, struct udp_pcb* upcb,  struct pbuf* p, 
													PCONST ip_addr_t* addr, uint16_t port) { 
	// Check port.
	if (port != this->port) { return; }
	
	// Process datagram here (non-blocking code).
	// Copy data to global buffer, set flag.
	ListNode* node = new ListNode;
	node->data = (char*) malloc(p->len);
	memcpy(node->data, p->payload, p->len);
	node->length = p->len;
	node->next = 0;
	if (response == 0) {
		// First response.
		response = node;
		last = node;
	}
	else {
		// Attach at end.
		last->next = node;
		last = node;
	}
	
	count++;
	received = true;
	
	// Free received pbuf before return.
	pbuf_free(p); 
} */


void NyanSD_client::udp_receive_callback(UdpConnection& connection, char* data, int size, 
													IpAddress remoteIP, uint16_t remotePort) {
	// Check IP address.
	if (remotePort != port) { return; }
	
	
	// Debug.
	// Dump data to serial port.
	Serial.println("=============================\r\n");
	Serial.println("Received message with length: " + String(size));
	Serial.println("=============================\r\n");	
	Serial.write(data, size);
	Serial.println("\r\n=============================\r\n");
	
	// Copy data to global buffer, set flag.
	ListNode* node = new ListNode;
	node->data = (char*) malloc(size);
	memcpy(node->data, data, size);
	node->length = size;
	node->next = 0;
	if (response == 0) {
		// First response.
		response = node;
		last = node;
	}
	else {
		// Attach at end.
		last->next = node;
		last = node;
	}
	
	count++;
	received = true;
}


// --- SEND QUERY ---
uint32_t NyanSD_client::sendQuery(uint16_t port, NYSD_query* queries, uint8_t qnum) {
										//ServiceNode* responses, uint32_t &resnum) {
	NyanSD_client::port = port;
											
	if (qnum > 255) {
		//std::cerr << "No more than 255 queries can be send simultaneously." << std::endl;
		return 1;
	}
	else if (qnum < 1) {
		//std::cerr << "At least one query must be sent. No query found." << std::endl;
		return 2;
	}
				
	// Compose the NYSD message.
	String msg = String("NYANSD");
	//char msg[] = { 'N', 'Y', 'A', 'N', 'S', 'D' };
	uint16_t len = 0;
	uint8_t type = (uint8_t) NYSD_MESSAGE_TYPE_BROADCAST;
	
	String body;
	body += String((char) qnum);
	NYSD_query* qptr = queries;
	for (int i = 0; i < qnum; ++i) {
		body += String("Q");
		uint8_t prot = (uint8_t) qptr->protocol;
		uint8_t qlen = (uint8_t) qptr->length;
		body += (char) prot;
		body += (char) qlen;
		if (qlen > 0) {
			body += qptr->filter;
		}
		
		if (i != (qnum - 1)) {
			++qptr;
		}
	}
	
	len = body.length() + 1;	// Add one byte for the message type.
	msg += String((char) *((char*) &len));
	msg += String((char) *(((char*) &len) + 1));
	msg += (char) type;
	msg += body;

		
	//err_t wr_err = ERR_OK;
	int length = msg.length();
	//udp_pcb* udpsocket = udp_new();
	//udp_recv(udpsocket, udp_receive_callback, 0);
	
	//UdpConnection udpsocket(udp_receive_callback);
	
	// Start listener on all ports.
 	response = 0;
	count = 0;
	udpsocket.listen(0);
	
	// Get broadcast IP.
	IpAddress bip = WifiStation.getIP();
	bip[3] = 255;
	
	/* wr_err = udp_bind(udpsocket, IP_ADDR_ANY, 0);
	wr_err = udp_connect(udpsocket, bip, port);
	if (wr_err != ERR_OK) { return 3; } */
	
	// Debug
	//IpAddress dip(192, 168, 31, 102);
	//String str("Nyan.");
	//uint16_t dport = 1234;
	/* if (!udpsocket.sendStringTo(dip, port, str)) {
		return 3;
	} */
	
	if (!udpsocket.sendStringTo(bip, port, msg)) {
		return 4;
	}
	
	// Debug
	//return 5;
	
	/* struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	memcpy(p->payload, msg.c_str(), length);
	//p->len = length;
	//p->tot_len = length;
	wr_err = udp_sendto(udpsocket, p, bip, port);
	if (wr_err != ERR_OK) {
		if (wr_err == ERR_MEM) { return 4; }
		else if (wr_err == ERR_RTE) { return 5; }
		else if (wr_err == ERR_VAL) { return 6; }
		//wr_err = udp_sendto(udpsocket,p, IP_ADDR_BROADCAST, 10);
		return 7;
	} */
	
	//pbuf_free(p);
	
	//udp_bind(udpsocket, IP_ADDR_ANY, 0);
	
	return 0;
}
		
		
uint32_t NyanSD_client::getResponses(ServiceNode* &responses, uint32_t &resnum) {
	// Set up the callback and start receiving.
	// Wait for the receive callback to be called. Wait for a maximum of 200 ms.
	//delay(5000);
	/* int timeout = 5000; // 5 seconds
	while (!received) {
		delay(1);
		timeout--;
		if (timeout < 1) { return 8; }
	} */
				
	// Close socket as we're done with this interface.
	//udpsocket.close();
	//udp_recv(udpsocket, nullptr, nullptr);
	//udp_remove(udpsocket);
	
	if (!received) {
		return 1;
	}
	
	// Copy parsed responses into the 'responses' list.
	uint32_t parsecount = 0;
	ServiceNode* pres = 0;
	for (int i = 0; i < count; ++i) {
		int n = response->length;
		if (n < 8) {
			// Nothing to do. Try next response, if any.
			// TODO: aborting parsing for now.
			return 8;
		}
		
		// Debug.
		// Dump data to serial port.
		Serial.println("=============================\r\n");
		Serial.write(response->data, response->length);
		Serial.println("\r\n=============================\r\n");
		
		// The received data can contain more than one response. Start parsing from the beginning until
		// we are done.
		char* buffer = response->data;
		//int index = 0;
		//while (index < n) {
			int index = 0;
			//std::string signature = std::string(buffer, 6);
			char signature[6];
			memcpy((char*) &signature, buffer, 6);
			index += 6;
			//if (signature != "NYANSD") {
			if (strncmp((char*) &signature, "NYANSD", 6) != 0) {
				//std::cerr << "Signature of message incorrect: " << signature << std::endl;
				return 9;
			}
			
			uint16_t len;
			memcpy(&len, (buffer + index), 2);
			index += 2;
			
			//if (len > buffers[i].length - (index)) {
			if (len > (response->length - index)) {
				//std::cerr << "Insufficient data in buffer to finish parsing message: " << len << "/" 
					//		<< (buffers[i].length - (index + 6)) << std::endl;
				return 10;
			}
			
#ifdef DEBUG
			//std::cout << "Found message with length: " << len << std::endl;
#endif

			Serial.println(_F("Message of length: ") + String(len));
			
			uint8_t type;
			type = *((uint8_t*) (buffer + index++));
			
#ifdef DEBUG
			//std::cout << "Message type: " << (uint16_t) type << std::endl;
#endif
			
			if (type != NYSD_MESSAGE_TYPE_RESPONSE) {
				//std::cerr << "Not a response message type. Skipping..." << std::endl;
				return 11;
			}
			
			// Number of services in this response.
			uint8_t rnum = *((uint8_t*) buffer + index++);
#ifdef DEBUG
			//std::cout << "Response count: " << (uint16_t) rnum << std::endl;
#endif
			Serial.println(_F("Response count: ") + String(rnum));
			
			// Service sections.
			for (int i = 0; i < rnum; ++i) {
				if (buffer[index] != 'S') {
					//std::cerr << "Invalid service section signature. Aborting parsing." << std::endl;
					return 12;
				}
				
				index++;
				uint32_t ipv4;
				memcpy(&ipv4, (buffer + index), 4);
				index += 4;
				
				Serial.println(_F("Service IPv4: ") + String(ipv4));
				
				uint8_t ipv6len = *((uint8_t*) (buffer + index));
				index++;
				
#ifdef DEBUG
				//std::cout << "IPv6 string with length: " << (uint16_t) ipv6len << std::endl;
#endif

				Serial.println(_F("IPv6 string of length: ") + String((uint16_t) ipv6len));
				
				char* ipv6 = new char[ipv6len];
				memcpy(ipv6, buffer + index, ipv6len);
				index += ipv6len;
				
				Serial.println(_F("IPv6:\t"));
				Serial.write(ipv6, ipv6len);
				Serial.println(".");
				
				uint16_t hostlen;
				memcpy(&hostlen, (buffer + index), 2);
				index += 2;
				
				char* hostname = new char[hostlen];
				memcpy(hostname, buffer + index, hostlen);
				index += hostlen;
				
				Serial.println(_F("Host name:\t"));
				Serial.write(hostname, hostlen);
				Serial.println(".");
				
				uint16_t p;	// port
				memcpy(&p, (buffer + index), 2);
				index += 2;
				
				uint8_t prot = *((uint8_t*) (buffer + index));
				index++;
				
				uint16_t snlen;
				memcpy(&snlen, (buffer + index), 2);
				index += 2;
				
				char* svname = new char[snlen];
				memcpy(svname, buffer + index, snlen);
				index += snlen;
				
#ifdef DEBUG
				//std::cout << "Adding service with name: " << svname << std::endl;
#endif
				Serial.println(_F("Adding service:\t"));
				Serial.write(svname, snlen);
				Serial.println(".");
				
				ServiceNode* sn = new ServiceNode;
				sn->next = 0;
				NYSD_service* sv = new NYSD_service;
				sv->ipv4 = ipv4;
				sv->ipv6 = ipv6;
				sv->port = p;
				sv->hostname = hostname;
				sv->service = svname;
				if (prot == NYSD_PROTOCOL_ALL) {
					sv->protocol = NYSD_PROTOCOL_ALL;
				}
				else if (prot == NYSD_PROTOCOL_TCP) {
					sv->protocol = NYSD_PROTOCOL_TCP;
				}
				else if (prot == NYSD_PROTOCOL_UDP) {
					sv->protocol = NYSD_PROTOCOL_UDP;
				}
				
				Serial.println(_F("Adding service to response..."));
				
				sn->service = sv;
				
				Serial.println(_F("Service IPv4: ") + String(ipv4));
				Serial.println(_F("Service IPv4: ") + String(sv->ipv4));
				Serial.println(_F("Service IPv4: ") + String(sn->service->ipv4));
				
				Serial.println(_F("Adding response to output list..."));
				
				if (parsecount == 0) {
					Serial.println(_F("First response."));
					responses = sn;
					pres = responses;
				}
				else {
					Serial.println(_F("Not first response."));
					pres->next = sn;
					pres = sn;
				}
				
				Serial.println(_F("Response count: ") + String(rnum));
				
				//resnum += 1;
				++parsecount;
				
				// Debug
				break;
			}
			
#ifdef DEBUG
			//std::cout << "Buffer: " << index << "/" << n << std::endl;
#endif
		//}
		
		// Delete previously allocated data.
		Serial.println(_F("Delete old response data."));
		free(buffer);
		
		// Prepare for next loop: move to next list node, delete old node.
		Serial.println(_F("Checking for next response..."));
		ListNode* old = response;
		if (response->next) { 
			Serial.println(_F("Moving to next response..."));
			response = response->next;
		}
		
		Serial.println(_F("Deleting old response..."));
		delete old;
	}
	
	Serial.println("Found responses: " + String(parsecount));
	
	resnum = parsecount;
	
	Serial.println("Found responses: " + String(resnum));
	
	return 0;
}


// -- IPv4
String NyanSD_client::ipv4_uintToString(uint32_t ipv4) {
	/* String out;
	for (int i = 0; i < 4; ++i) {
		//out += std::to_string(*(((uint8_t*) &ipv4) + i));
		int ch = *(((uint8_t*) &ipv4) + i);
		out.concat(('0' + ch));
		if (i < 3) { out += "."; }
	} */
	
	uint8_t* p = (uint8_t*) &ipv4;
	uint8_t o0 = *p;
	p++;
	uint8_t o1 = *p;
	p++;
	uint8_t o2 = *p;
	p++;
	uint8_t o3 = *p;
	IpAddress addr(o0, o1, o2, o3);
	String out = addr.toString();
	
	return out;
}
