/*
	nyansd_client.h - Header file for the NyanSD service discovery client for Arduino.
	
	Notes:
			- Adapted from the full-fat version, with some simplifications.
			- Assumes usage on a little-endian platform.
			
	2020/04/23, Maya Posch
	2022/08/09, Maya Posch - Created Arduino client version.
*/


#ifndef NYANSD_CLIENT_H
#define NYANSD_CLIENT_H


/* #include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <string> */

#include <cstdint>

#include <lwip/ip_addr.h>

#include <SmingCore.h>


enum NYSD_message_type {
	NYSD_MESSAGE_TYPE_BROADCAST	= 0x01,
	NYSD_MESSAGE_TYPE_RESPONSE	= 0x02
};


enum NYSD_protocol {
	NYSD_PROTOCOL_ALL = 0x00,
	NYSD_PROTOCOL_TCP = 0x01,
	NYSD_PROTOCOL_UDP = 0x02
};


struct NYSD_service {
	uint32_t ipv4 = 0;
	char* ipv6;
	uint16_t port = 0;
	char* hostname;
	char* service;
	NYSD_protocol protocol = NYSD_PROTOCOL_ALL;
};


struct NYSD_query {
	NYSD_protocol protocol;
	char* filter;
	uint16_t length;
};


struct ListNode {
	ListNode* next;
	char* data;
	int length;
};


struct ServiceNode {
	NYSD_service* service;
	ServiceNode* next;
};


#if defined __linux__ || defined _WIN32
	#define PCONST const
#else
	#define PCONST
#endif


extern UdpConnection udpsocket;


class NyanSD_client {
	//static std::vector<NYSD_service> services;
	//static std::mutex servicesMutex;
	//static std::atomic<bool> running;
	//static std::thread handler;
	//static ByteBauble bb;
	
	static ListNode* response;
	static ListNode* last;
	static int count;
	static bool received;
	static uint16_t port;
	static UdpConnection udpsocket;
	
	//static void clientHandler(uint16_t port);
	
public:
	
	/* static void udp_receive_callback(void* arg, struct udp_pcb* upcb,  struct pbuf* p, 
													PCONST ip_addr_t* addr, uint16_t port); */
	static void udp_receive_callback(UdpConnection& connection, char* data, int size, 
													IpAddress remoteIP, uint16_t remotePort);
	/*static bool sendQuery(uint16_t port, std::vector<NYSD_query> queries, 
										std::vector<NYSD_service> &responses);*/
	static uint32_t sendQuery(uint16_t port, NYSD_query* queries, uint8_t qnum);
	static uint32_t getResponses(ServiceNode* &responses, uint32_t &resnum);
	static bool hasResponse() { return (count > 0) ? true : false; }
	//static bool addService(NYSD_service service);
	//static bool startListener(uint16_t port);
	//static bool stopListener();
	
	static String ipv4_uintToString(uint32_t ipv4);
	//static uint32_t ipv4_stringToUint(std::string ipv4);
};


#endif
