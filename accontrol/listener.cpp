/*
	influx_mqtt.cpp - Proxy to convert MQTT events into InfluxDB HTTP protocol.
	
	Revision 0
	
	Features:
				- 
				
	Notes:
				-
				
	2017/02/09, Maya Posch <posch@synyx.de>
*/

#include "listener.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Thread.h>
#include <Poco/NumberFormatter.h>

using namespace Poco;


// Constants
const char GPIOPins[] = { 0x0e, 0x0d, 0x0c };


// --- CONSTRUCTOR ---
Listener::Listener(string clientId, string host, int port) : mosquittopp(clientId.c_str()) {
	int keepalive = 60;
	heating = false;
	connect(host.c_str(), port, keepalive);

	// Done for now.
}


// --- DECONSTRUCTOR ---
Listener::~Listener() {
	//
}


// --- ON CONNECT ---
void Listener::on_connect(int rc) {
	cout << "Connected. Subscribing to topics...\n";
	
	// Check code.
	if (rc == 0) {
		// Subscribe to desired topics.
		string topic = "pwm/response";	// AC controller responses.
		subscribe(0, topic.c_str(), 1);
		topic = "io/response/#";	// AC controller responses.
		subscribe(0, topic.c_str(), 1);
		topic = "switch/response/#";	// AC controller responses.
		subscribe(0, topic.c_str(), 1);
	}
	else {
		// handle.
		cerr << "Connection failed. Aborting subscribing.\n";
	}
}


// --- ON MESSAGE ---
void Listener::on_message(const struct mosquitto_message* message) {
	string topic = message->topic;
	string payload = string((const char*) message->payload, message->payloadlen);
	
	if (topic == "pwm/response") {
		// Payload is the node UID (string) followed by a semi-colon and the
		// rest of the payload.
		StringTokenizer st(payload, ";");
		if (st.count() < 2) {
			cerr << "PWM message: Wrong number of arguments. Payload: " << payload << "\n";
			return; 
		}
		
		//cout << "Payload: " << payload << endl;
		//cout << "Binary: " << std::hex << *((UInt32*) st[1].c_str()) << endl;
		
		string uid = st[0];
		
		nodesLock.lock();
		map<string, NodeInfo>::iterator it;
		it = nodes.find(uid);
		if (it == nodes.end()) {
			cerr << "Unknown UID. Skipping.\n";
			nodesLock.unlock();
			return;
		}
		
		cout << "PWM receive for UID " << uid << ", validate: " << (UInt32) it->second.validate << endl;
		
		if (it->second.validate == 0) {
			// We should have received a list of the active pins:
			// <uint8*>
			string pins = st[1];
			
			// If the list is empty, the node needs to be initialised.
			// For now we assume three connected channels (0xc, 0xd, 0xe).
			// TODO: use saved channel info for each node for intelligent queries.
			if (pins.empty()) {
				cout << "Initializing UID: " << uid << endl;
				
				string topic = "pwm/" + uid;
				char initAll[] = { 0x01, 0x03, 0x0c, 0x0d, 0x0e };
				publish(0, topic.c_str(), 5, initAll, 1); // QoS 1.
				
				// Pause for half a second while the node initialises.
				Thread::sleep(500);
			}
			//else if (st[1].compare(GPIOPins) == 0) {
			else if (pins.length() == 3) {
				// All pins are active. We can continue.
				it->second.validate = 1;
				
				// Set state of channel validation level.
				it->second.ch0_valid = false;
				it->second.ch1_valid = false;
				it->second.ch2_valid = false;
				it->second.ch3_valid = false;
				
				// Request the duty cycle for each active pin.
				string topic = "pwm/" + uid;
				char ch0duty[] = { 0x08, 0x0c };
				char ch1duty[] = { 0x08, 0x0d };
				char ch2duty[] = { 0x08, 0x0e };
				char ch3duty[] = { 0x08, 0x0f };
				publish(0, topic.c_str(), 2, ch0duty, 1); // QoS 1
				publish(0, topic.c_str(), 2, ch1duty, 1); // QoS 1
				publish(0, topic.c_str(), 2, ch2duty, 1); // QoS 1
				publish(0, topic.c_str(), 2, ch3duty, 1); // QoS 1
			}
			else {				
				// The node should be initialised now. 
				// Look for the '1' confirmation.
				if (st[1] != "1") {
					cerr << "Initializing node '" + uid + "' failed. Skipping.\n";
					nodesLock.unlock();
					return;
				}
				
				// Resend the request for the active pins.
				string topic = "pwm/" + uid;
				char payload[] = { 0x10 };
				publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
			}
		}
		
		if (it->second.validate == 1) {
			// The node is active, but we need to validate the state of its
			// PWM outputs.
			// Here we receive answers from nodes containing the current duty 
			// for a specific pin:
			// uint8	GPIO number.
			// uint8	Duty level (1 - 6).
			//
			// Duty level returned from the node is the set level + 1 for 
			// technical reasons. Subtract one when comparing with the NodeInfo
			// data.
			string res = st[1];
			if (res.length() != 2) {
				cerr << "Received wrong response length from node for duty level. Skipping.\n";
				nodesLock.unlock();
				return;
			}
			
			UInt8 ch = (UInt8) res[0];
			int count = 0;
			if (it->second.ch0_valid) { ++count; }
			if (it->second.ch1_valid) { ++count; }
			if (it->second.ch2_valid) { ++count; }
			if (it->second.ch3_valid) { ++count; }
			
			cout << "Validating channel: " << std::hex << (UInt32) ch << endl;
			
			string topic = "pwm/" + uid;
			if (count == 4) {
				// We're done.
				it->second.validate = 2;
			}
			else if (ch == 0x0c) { // ch1
				// Validate this channel.
				UInt8 duty = (UInt8) res[1];
				if ((duty - 1) == it->second.ch1_duty) { it->second.ch1_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					unsigned char payload[] = { 0x04, 0x0c, it->second.ch1_duty };
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					it->second.ch1_valid = true;
					++count;
				}
			}
			else if (ch == 0x0d) { // ch2
				// Validate this channel.
				UInt8 duty = (UInt8) res[1];
				if ((duty - 1) == it->second.ch2_duty) { it->second.ch2_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					unsigned char payload[] = { 0x04, 0x0d, it->second.ch1_duty };
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					it->second.ch2_valid = true;
					++count;
				}				
			}
			else if (ch == 0x0e) { // ch0
				// Validate this channel.
				UInt8 duty = (UInt8) res[1];
				if ((duty - 1) == it->second.ch0_duty) { it->second.ch0_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					unsigned char payload[] = { 0x04, 0x0e, it->second.ch0_duty };
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					it->second.ch0_valid = true;
					++count;
				}
			}
			else if (ch == 0x0f) { // ch3
				// Validate this channel.
				UInt8 duty = (UInt8) res[1];
				if ((duty - 1) == it->second.ch3_duty) { it->second.ch3_valid = true; ++count; }
				else {
					// Ask the node to change its duty cycle.
					unsigned char payload[] = { 0x04, 0x0f, it->second.ch3_duty };
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
					it->second.ch3_valid = true;
					++count;
				}
			}
			
			if (count == 4) {
				// We're done.
				it->second.validate = 2;
			}
		}
		
		if (it->second.validate == 2) {
			// Node has been validated. Adjust the settings for the node based
			// on the target & current temperature.
			//
			// The measured (current) temperature is measured at the ceiling.
			// To compensate for this we subtract 1C from it.
			float delta = (it->second.current - 1) - it->second.target;
			
			// Our delta tells us whether it's too warm (positive value), or
			// too cold (negative value). This tells us which channels to change.
			//
			// If we're cooling, we need to invert the delta to get the 
			// appropriate response.
			if (heating) {
				delta = delta * -1;
			}
			
			cout << "Current temperature delta: " << delta << endl;
			
			//
			// An important factor here is whether the AC unit (fan coil unit, 
			// FCU) is set to cool or heat. If it's too warm, but the system is 
			// set to heating, then there's nothing we can do with the fans.
			//
			// We can switch the entire system in a section from heating to 
			// cooling using the appropriate switch.
			
			//
			if (delta > 4.0) {
				// All to level 5.
				Nodes::setDuty(uid, 5, 5, 5, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 3.0) {
				// All to level 4.
				Nodes::setDuty(uid, 4, 4, 4, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 2.0) {
				// All to level 3.
				Nodes::setDuty(uid, 3, 3, 3, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 1.0) {
				// All to level 2.
				Nodes::setDuty(uid, 2, 2, 2, 0);
				
				// Open the valves on all units.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 0.8) {
				// Open the valve on all units.
				//Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 0.6) {
				// All to level 1.
				Nodes::setDuty(uid, 1, 1, 1, 0);
				
				// All valves open.
				Nodes::setValves(uid, true, true, true, false);
			}
			else if (delta > 0.4) {
				// Channels 0 and 1 to level 1.
				Nodes::setDuty(uid, 1, 1, 0 , 0);
				
				// Open first two valves.
				Nodes::setValves(uid, true, true, false, false);
			}
			else if (delta > 0.2) {
				// Channel 1 ('center') to 1.
				Nodes::setDuty(uid, 0, 1, 0, 0);
				
				// Open center valve.
				Nodes::setValves(uid, false, true, false, false);
			}
			else if (delta < -3.0) {
				// Switch the system from the current mode (cooling/heating) to
				// the opposite mode in order to regain effectiveness.
				// FIXME: hard-coding switch ID. Make dynamic.
				Nodes::setSwitch("sw-grossraum", !heating);
				checkSwitch();
			}
			else {
				// All fans off.
				Nodes::setDuty(uid, 0, 0, 0, 0);
				
				// Close all valves.
				Nodes::setValves(uid, false, false, false, false);
			}

			it->second.validate = 3;
		}
			
		nodesLock.unlock();
	}
	else if (topic.compare(0, 11, "io/response") == 0) {
		StringTokenizer st(payload, ";");
		if (st.count() < 2) {
			cerr << "I/O message: Wrong number of arguments. Payload: " 
					<< payload << "\n";
			return; 
		}
		
		string uid = st[0];
		
		// Check the command we get a response to and respond appropriately.
		if (st[1].length() < 2) {
			cerr << "I/O message: response with fewer than two parameters.\n";
			return;
		}
		
		UInt8 cmd = (UInt8) st[1][0];
		if (cmd == 0x01) {	// Start.
			if (st[1][1] != 0x01) {
				cerr << "I/O: failed to start node " << uid << endl;
				return;
			}
			
			// Request the current state.
			string topic = "io/" + uid;
			char payload[] = { 0x04 };
			publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
		}
		else if (cmd == 0x02) { // Stop.
			// Nothing.
		}
		else if (cmd == 0x04) { // State.
			if (st[1].length() != 5) {
				cerr << "I/O: Received corrupted I/O state message response.\n";
				cerr << "I/O: Length of state message was " << st[1].length() 
						<< " bytes.\n";
				return;
			}
			
			valvesLock.lock();
			map<string, ValveInfo>::iterator it;
			it = valves.find(uid);
			if (it == valves.end()) {
				cerr << "Unknown UID. Skipping.\n";
				valvesLock.unlock();
				return;
			}
			
			// Compare the valve settings with the desired settings.
			string topic = "io/" + uid;
			UInt8 gpio = (UInt8) st[1][4];
			if (it->second.ch0_valve) {
				// FIXME: we're just getting 0x00 back for each register at this
				// point; ignore the value for now and explicitly set the pin.
				//if (!(gpio & 0x1)) {
					char payload[] { 0x20, 0x01, 0x01 }; // Turn on valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x1)) {
					char payload[] { 0x20, 0x01, 0x00 }; // Turn off valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			
			if (it->second.ch1_valve) {
				//if (!(gpio & 0x2)) {
					char payload[] { 0x20, 0x02, 0x01 }; // Turn on valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x2)) {
					char payload[] { 0x20, 0x02, 0x00 }; // Turn off valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}	
			
			if (it->second.ch2_valve) {
				//if (!(gpio & 0x4)) {
					char payload[] { 0x20, 0x03, 0x01 }; // Turn on valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x4)) {
					char payload[] { 0x20, 0x03, 0x00 }; // Turn off valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}	
			
			if (it->second.ch3_valve) {
				//if (!(gpio & 0x8)) {
					char payload[] { 0x20, 0x04, 0x01 }; // Turn on valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}
			else {
				//if ((gpio & 0x8)) {
					char payload[] { 0x20, 0x04, 0x00 }; // Turn off valve.
					publish(0, topic.c_str(), 3, payload, 1); // QoS 1
				
					// Pause for half a second while the node settles.
					Thread::sleep(500);
				//}
			}					
			
			valvesLock.unlock();
		}
		else if (cmd == 0x08) { // Set mode.
			// All pins are set to 'output' mode by default. Nothing to do here.
		}
		else if (cmd == 0x10) { // Pull-up.
			// Nothing.
		}
		else if (cmd == 0x20) { // Write.
			if (st[1][1] != 0x01) {
				cerr << "I/O: failed to write pin on node " << uid << endl;
				return;
			}
		}
		else if (cmd == 0x40) { // Read.
			// Nothing.
		}
		else if (cmd == 0x80) { // Status.
			// Active status response. If 0x0, activate.
			// If active (0x1), start validation of settings.
			if (st[1].length() == 2) {
				if (st[1][1] == 0x0) {
					cerr << "I/O: error requesting active status.\n";
					return;
				}
				else {
					cerr << "I/O: corrupted active status response.\n";
					cerr << "I/O: received: ";
					for (uint i = 0; i < st[1].length(); ++i) {
						cerr << " "
							<< NumberFormatter::formatHex((UInt8) st[1][i], true); 
					}
					
					cerr << endl;
					return;
				}
			}
			else if (st[1].length() == 3) {
				if (st[1][1] != 0x01) {
					cerr << "I/O: active status reported failure.\n";
					return;
				}
				
				if (st[1][2] == 0x0) {
					// Initialise.
					string topic = "io/" + uid;
					char payload[] = { 0x01 };
					publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
				}
				else if (st[1][2] == 0x01) {
					// Validate.
					string topic = "io/" + uid;
					char payload[] = { 0x04 };
					publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
				}
				else {
					cerr << "I/O: invalid active status response value.\n";
					return;
				}
			}
		}
		else {
			cerr << "I/O message: unknown command.\n";
			return;
		}
	}
	else if (topic.compare(0, 15, "switch/response") == 0) {
		// We receive either the current position of the switch here, or a 
		// success/failure message for the changing of the switch's position.
		StringTokenizer st(payload, ";");
		if (st.count() < 2) {
			cerr << "Switch message: Wrong number of arguments. Payload: " << payload << "\n";
			return; 
		}
		
		string uid = st[0];
		
		// FIXME: currently we assume that we just have a single switch and thus
		// a single heating/cooling status. 
		// Read out payload value and set global variable.
		if (st[1].length() < 2) {
			cerr << "Switch message: response with fewer than two parameters.\n";
			return;
		}
		
		switchesLock.lock();
		map<string, SwitchInfo>::iterator it;
		it = switches.find(uid);
		if (it == switches.end()) {
			cerr << "Unknown UID. Skipping.\n";
			switchesLock.unlock();
			return;
		}
		
		if (st[1][0] == 0x04) {
			// Response containing the currently active pin.
			if (st[1].length() != 3) {
				cerr << "Switch message: wrong number of parameters for state response.";
				switchesLock.unlock();
				return;
			}
			
			// Validate switch state.
			UInt8 pin = (UInt8) st[1][2];
			if (pin == 0x00) { // Cooling state.
				if (it->second.state) {
					// Switch to heating.
					string topic = "switch/" + uid;
					char payload[] = { 0x02 };
					publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
				}
				else {
					cout << "Switch: confirming status as 'cooling'\n";
					heating = false;
				}
			}
			else if (pin == 0x01) { 
				if (!(it->second.state)) {
					// Switch to cooling.
					string topic = "switch/" + uid;
					char payload[] = { 0x01 };
					publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
				}
				else {
					cout << "Switch: confirming status as 'heating'\n";
					heating = true;
				}
			}
			else {
				cerr << "Switch message: received invalid switch state." << endl;
				switchesLock.unlock();
				return; 
			}
			
		}
		else if (st[1][0] == 0x01) {
			// Switch 1 (cooling position). Check return value.
			if (st[1][1] != 0x01) {
				// Command didn't succeed.
				switchesLock.unlock();
				return;
			}
			
			cout << "Switch: setting status to 'cooling'\n";
			heating = false;
		}
		else if (st[1][0] == 0x02) {
			// Switch 2 (heating position). Check return value.
			if (st[1][1] != 0x01) {
				// Command didn't succeed.
				switchesLock.unlock();
				return;
			}
			
			cout << "Switch: setting status to 'heating'\n";
			heating = true;
		}
		
		switchesLock.unlock();
	}
}


// --- ON SUBSCRIBE ---
void Listener::on_subscribe(int mid, int qos_count, const int* granted_qos) {
	// Report success, with details.
}


// --- CHECK NODES ---
// Check the PWM and I/O status for each node: active pins, current duty.
// Adjust active pins and duty cycle as needed.
bool Listener::checkNodes() {
	
	cout << "Listener::checkNodes() called.\n";
	
	// Get the info on each node.
	vector<string> uids;
	if (!Nodes::getUIDs(uids)) { return false; }
	
	cout << "Checking nodes: " << uids.size() << endl;
	
	int uidsl = uids.size();
	//nodes.clear();
	nodesLock.lock();
	for (unsigned int i = 0; i < uidsl; ++i) {
		// Start the validation sequence for this node.
		NodeInfo info;
		if (!Nodes::getNodeInfo(uids[i], info)) {
			cerr << "Error getting info for node " << uids[i] << ". Skipping...\n";
			continue;
		}
		
		ValveInfo vinfo;
		if (!Nodes::getValveInfo(uids[i], vinfo)) {
			cerr << "Error getting valve info for node " << uids[i] << ". Skipping...\n";
			continue;
		}
		
		// Store info for this node.
		info.validate = 0; // starting a new validation cycle.
		nodes[uids[i]] = info;
		valves[uids[i]] = vinfo;
		
		// Request list of active pins from the node.
		string topic = "pwm/" + uids[i];
		char payload[] = { 0x10 };
		publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
		
		// Request current valve status from the node.
		topic = "io/" + uids[i];
		unsigned char pl[] = { 0x80 };
		publish(0, topic.c_str(), 1, pl, 1);
	}
	
	nodesLock.unlock();
	return true;
}


// --- CHECK SWITCH ---
// Request the status of the heating/cooling switch.
bool Listener::checkSwitch() {
	cout << "Listener::checkSwitch() called.\n";
	
	// Get the info on each node.
	vector<string> uids;
	if (!Nodes::getSwitchUIDs(uids)) { return false; }
	
	int uidsl = uids.size();
	switchesLock.lock();
	for (unsigned int i = 0; i < uidsl; ++i) {
		SwitchInfo info;
		if (!Nodes::getSwitchInfo(uids[i], info)) {
			cerr << "Error getting info for switch " << uids[i] << ". Skipping...\n";
			continue;
		}
		
		// Store info for this switch.
		switches[uids[i]] = info;
		
		// Sync the system state with the stored state.
		heating = info.state;
		
		// Send status request to switch.
		string topic = "switch/" + uids[i];
		char payload[] = { 0x04 };
		publish(0, topic.c_str(), 1, payload, 1); // QoS 1.
	}
	
	switchesLock.unlock();
}
