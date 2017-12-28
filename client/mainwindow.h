#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QMetaType>

#include "mqttlistener.h"
#include "nodetextitem.h"


Q_DECLARE_METATYPE(NodeTextItem*)
Q_DECLARE_METATYPE(Node)
Q_DECLARE_METATYPE(QVector<Node>)


namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow {
    Q_OBJECT
    
private slots:
    void registerNode();
    void connectRemote();
    void updateMap();
    void updateNodes();
    void saveNode();
    void updateNode();
    void nodeSelected(NodeTextItem* nodeTextItem);
    void setMap(QPixmap map);
    void setNodes(QVector<Node> nodes);
    void deleteNode();
    void errorHandler(QString err);
    void saveConfiguration();
    void showFirmwareDialogue();
    void showNodeFirmwareDialogue();
    void sendMessage(string topic, string message);
    void quit();
    
    void uidChanged();
    void locationChanged();
    void positionXChanged(double value);
    void positionYChanged(double value);
    void modulesChanged(int state);
    void itemMoved(float posx, float posy);
    
signals:
    void uidChanged(QString uid);
    void locationChanged(QString location);
    void positionXChanged(float x);
    void positionYChanged(float y);
    void modulesChanged(quint32 modules);
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    bool serializeNode(string &data);
    
    Ui::MainWindow* ui;
    bool connected;
    QString remoteServer;
    int remotePort;
    MqttListener* mqtt;
    NodeTextItem* nodeItem;
    string ca, cert, key;
    bool savedNode;
};

#endif // MAINWINDOW_H
