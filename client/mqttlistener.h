#ifndef MQTTLISTENER_H
#define MQTTLISTENER_H


#include <mosquittopp.h>

#include <string>

using namespace std;

#include <QObject>
#include <QPixmap>


// Enums
enum {
	MOD_TEMPERATURE_HUMIDITY = 0x1,
	MOD_CO2 = 0x2,
	MOD_JURA = 0x4,
	MOD_JURATERM = 0x8,
    MOD_MOTION = 0x10,
    MOD_PWM = 0x20,
    MOD_IO = 0x40,
    MOD_SWITCH = 0x80
};


// Structs
struct Node {
	string uid;
	string location;
	quint32 modules;
	float posx;
	float posy;
    bool saved;
};


class MqttListener : public QObject, mosqpp::mosquittopp {
    Q_OBJECT
    
    QString host;
    int port;
    
public:
    explicit MqttListener(QObject *parent = 0, QString clientId = "CC-UI", 
                          QString host = "localhost", int port = 1883);
    ~MqttListener();
    
    bool setTLS(string &ca, string &cert, string &key);
    void on_connect(int rc);
    void on_message(const struct mosquitto_message* message);
    void on_subscribe(int mid, int qos_count, const int* granted_qos);
    void on_log(int level, const char* str);
    void publishMessage(string topic, string msg);
    
signals:
    void connected();
    void failed(QString err);
    void receivedNodes(QVector<Node> nodes);
    void receivedMap(QPixmap image);
    void receivedFirmwareList(QString list);
    void finished();
    
public slots:
    void connectBroker();
};

#endif // MQTTLISTENER_H
