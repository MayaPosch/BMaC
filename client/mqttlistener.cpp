#include "mqttlistener.h"

#include <iostream>

using namespace std;

#include <QDataStream>
#include <QVector>


// --- CONSTRUCTOR ---
// Connect to the MQTT broker using the provided host and port details.
MqttListener::MqttListener(QObject *parent, QString clientId, QString host, int port) : QObject(parent), mosquittopp(clientId.toStdString().c_str()) {
    this->host = host;
    this->port = port;
}


// --- DECONSTRUCTOR ---
MqttListener::~MqttListener() {   
    mosqpp::lib_cleanup();
    emit finished();
}


// --- SET TLS ---
// Set the CA, Cert and Key file for TLS connections.
bool MqttListener::setTLS(string &ca, string &cert, string &key) {
    cout << "CA: " << ca << ", cert: " << cert << ", key: " << key << ".\n";
        
    int res = tls_set(ca.c_str(), 0, cert.c_str(), key.c_str(), 0);
    if (res != MOSQ_ERR_SUCCESS) { 
        if (res == MOSQ_ERR_INVAL) {
            cerr << "Invalid input parameters for Mosquitto TLS.\n";
        }
        else if (res == MOSQ_ERR_NOMEM) {
            cerr << "Out of memory on Mosquitto TLS.\n";
        }
        else {
            cerr << "Unknown Mosquitto TLS error.\n";
        }
        
        return false; 
    }
    
    
    tls_opts_set(0, 0, 0);  // SSL_VERIFY_NONE
    tls_insecure_set(1);    // insecure connect.
    
    return true;
}


// --- CONNECT BROKER ---
void MqttListener::connectBroker() {
    cout << "Connecting to broker..." << host.toStdString() << ":" << port << "\n";
    int keepalive = 60;
    mosqpp::mosquittopp::connect(host.toStdString().c_str(), port, keepalive);
    
    int rc;
    while (1) {
        rc = loop();
        if (rc) {
            if (rc == MOSQ_ERR_INVAL) {
                cerr << "Loop: received ERR_INVAL error.\n";
            }
            else if (rc == MOSQ_ERR_NO_CONN) {
                cerr << "Loop: received ERR_NO_CONN.\n";
            }
            else if (rc == MOSQ_ERR_SUCCESS) {
                cerr << "Loop: received ERR_SUCCESS.\n";
            }
            else if (rc == MOSQ_ERR_ERRNO) {
                cerr << "Loop: received ERR_ERRNO.\n";
#ifdef WIN32
                // TODO: Use FormatMessage() to get the error string.
                cerr << "Loop: received error: " << strerror(errno) << endl;
#else
                // TODO: Use strerror_r() to get the error string.
                cerr << "Loop: system error: " << strerror(errno) << endl;
#endif
            }
            else {
                cerr << "Loop: received error: " << rc << endl;
            }
            
            emit failed("Failed to connect to remote server.");
            break;
            //cout << "Disconnected. Trying to reconnect...\n";
            //reconnect();
        }
    }
}


// --- ON CONNECT ---
void MqttListener::on_connect(int rc) {
    cout << "Connected.\n";
            
    if (rc == 0) {
        cout << "Subscribing to topics...\n";
        
        // Subscribe to topics.
        string topic = "cc/ui/config/map";
        subscribe(0, topic.c_str());
        topic = "cc/nodes/all";
        subscribe(0, topic.c_str());
        topic = "cc/firmware/list";
        subscribe(0, topic.c_str());
        
        emit connected();
    }
    else {
		// handle.
		cerr << "Connection failed. Aborting subscribing.\n";
        emit failed("Connecting failed. Aborting subscribing");
	}
}


// --- ON SUBSCRIBE ---
void MqttListener::on_subscribe(int mid, int qos_count, const int *granted_qos) {
    // 
}


// --- ON MESSAGE ---
void MqttListener::on_message(const mosquitto_message *message) {
    string topic = message->topic;
	string payload = string((const char*) message->payload, message->payloadlen);
    
    if (topic == "cc/ui/config/map") {
        // We received the current map image. Process it and set it in the UI.
        QPixmap img;
        if (!img.loadFromData((const uchar*) payload.data(), payload.length())) {
            // Failed to load image.
            cerr << "Failed to parse map image data. Aborting.\n";
            //
        }
        
        // Send the image to the UI.
        emit receivedMap(img);
    }
    else if (topic == "cc/nodes/all") {
        cout << "Received nodes message from the server.\n";
        
        // We received a list of the current nodes. Process it and set it in the UI.
        // If node data already exists, ask whether to replace it.
        // Format:
        // Header:
        // uint64	total message size following this integer.
        // uint8(5)	"NODES" in ASCII
        // uint32	Number of node segments
        // <node segments>
        //
        // Segment:
        // Payload format (LE direction):
        // uint32	size of the segment after this integer.
        // uint8(4)	"NODE" in ASCII
        // uint8	UID length
        // uint8(*)	UID string
        // uint8	location length
        // uint8(*)	location string
        // float32	Position X
        // float32	Position Y
        // uint32	Active modules (bit flags)
        quint32 index = 0;
        quint64 msgSize = *((quint64*) payload.substr(index, 8).data());
        index += 8;
        string signature = payload.substr(index,5);
        index += 5;
        quint32 numNodes = *((quint32*) payload.substr(index, 4).data());
        index += 4;
        
        cout << "Message size: " << msgSize << "\n";
        cout << "Signature: " << signature << "\n";
        cout << "Nodes: " << numNodes << "\n";
        
        if (signature != "NODES") { 
            cerr << "False signature: 'NODES' not found: " << signature << "\n";
            return; 
        }
        
        QVector<Node> nodes;
        for (int i = 0; i < numNodes; ++i) {
            quint32 sectionLength = *((quint32*) payload.substr(index, 4).data());
            index += 4;
            string signature = payload.substr(index, 4);
            index += 4;
            if (signature != "NODE") {
                cerr << "False signature: 'NODE' not found: " << signature << "\n";
                break; 
            }
            
            quint8 uidLength = (quint8) payload[index++];
            Node node;
            node.uid = payload.substr(index, uidLength);
            index += uidLength;
            quint8 locationLength = (quint8) payload[index++];
            node.location = payload.substr(index, locationLength);
            index += locationLength;
            node.posx = *((float*) payload.substr(index, 4).data());
            index += 4;
            node.posy = *((float*) payload.substr(index, 4).data());
            index += 4;
            node.modules = *((quint32*) payload.substr(index, 4).data());
            index += 4;
            node.saved = true;
            
            cout << "Found node: " << node.uid << "\n";
            
            nodes.append(node);
        }
        
        // TODO: ask to replace if data already exists.
        
        // Send the received data to the UI.
        emit receivedNodes(nodes);
    }
    else if (topic == "cc/firmware/list") {
        // We should have received a semi-colon-separated list of firmware
        // filenames. Pass it on to any subscribing slots.
        QString msgList = QString::fromStdString(payload);
    }
}


// --- ON LOG ---
void MqttListener::on_log(int level, const char *str) {
    cout << "LOG: " << str << ".\n";
}


// --- PUBLISH MESSAGE ---
void MqttListener::publishMessage(string topic, string msg) {
    publish(0, topic.c_str(), msg.length(), msg.c_str());
}
