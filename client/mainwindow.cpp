#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "firmwaredialogue.h"
#include "nodefirmwaredialogue.h"

//#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QThread>
#include <QFile>

#include <iostream>

using namespace std;


// --- CONSTRUCTOR ---
MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    
    // UI connections.
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quit()));
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(connectRemote()));
    connect(ui->actionImages, SIGNAL(triggered()), this, SLOT(showFirmwareDialogue()));
    connect(ui->actionNodes, SIGNAL(triggered()), this, SLOT(showNodeFirmwareDialogue()));
    connect(ui->saveNodeButton, SIGNAL(pressed()), this, SLOT(saveNode()));
    connect(ui->updateNodeButton, SIGNAL(pressed()), this, SLOT(updateNode()));
    connect(ui->deleteNodeButton, SIGNAL(pressed()), this, SLOT(deleteNode()));
    
    connect(ui->NodeUidEdit, SIGNAL(editingFinished()), this, SLOT(uidChanged()));
    connect(ui->nodeLocationEdit, SIGNAL(editingFinished()), this, SLOT(locationChanged()));
    connect(ui->posxSpinBox, SIGNAL(valueChanged(double)), this, SLOT(positionXChanged(double)));
    connect(ui->posySpinBox, SIGNAL(valueChanged(double)), this, SLOT(positionYChanged(double)));
    connect(ui->tempHumidityCheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    connect(ui->switchCheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    connect(ui->ioCheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    connect(ui->co2CheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    connect(ui->pwmCheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    connect(ui->JuraCheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    connect(ui->juraTermCheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    connect(ui->motionCheckBox, SIGNAL(stateChanged(int)), this, SLOT(modulesChanged(int)));
    
    // Read configuration.
    QCoreApplication::setApplicationName("C&C UI");
    QCoreApplication::setApplicationVersion("v0.1");
    QCoreApplication::setOrganizationName("Nyanko");
    QCoreApplication::setOrganizationDomain("www.nyanko.ws");
    QSettings settings;
    remoteServer = settings.value("lastRemote", "localhost").toString();
    remotePort = settings.value("lastPort", 1883).toInt();
    ca = settings.value("caPath", "").toString().toStdString();
    cert = settings.value("certPath", "").toString().toStdString();
    key = settings.value("keyPath", "").toString().toStdString();
    
    // Add toolbar actions.
    ui->mainToolBar->addAction("New node", this, SLOT(registerNode()));
    
    // Defaults.
    mqtt = 0;
    nodeItem = 0;
    connected = false;
    savedNode = false;
    
    // Register meta type. This is required to make signal slot passing work.
    qRegisterMetaType<NodeTextItem*>();
    qRegisterMetaType<QVector<Node> >();
    qRegisterMetaType<Node>();
}


// --- DECONSTRUCTOR ---
MainWindow::~MainWindow() {
    delete ui;
    if (mqtt) { delete mqtt; }
}


// --- UPDATE MAP ---
// Update the displayed map for the remote C&C server.
void MainWindow::updateMap() {
    // Publish the MQTT request for the map data.
    connected = true;
    mqtt->publishMessage("cc/ui/config", "map");
}


// --- UPDATE NODES ---
// Update the nodes from the remote C&C server.
void MainWindow::updateNodes() {
    // Publish the request to receive the nodes.
    connected = true;
    mqtt->publishMessage("cc/ui/config", "nodes");
}


// --- UPDATE NODE ---
// Sends the updated configuration to the target node (UID).
// This does not save the configuration on the C&C server.
void MainWindow::updateNode() {
    if (!connected) {
        QMessageBox::warning(this, tr("Not connected"), tr("Not connected to MQTT server."));
        return;
    }
    
    Node node;
    node.uid = ui->NodeUidEdit->text().toStdString();
    node.location = ui->nodeLocationEdit->text().toStdString();
    
    bool tnh = ui->tempHumidityCheckBox->isChecked();
    bool pwm = ui->pwmCheckBox->isChecked();
    bool io = ui->ioCheckBox->isChecked();
    bool sw = ui->switchCheckBox->isChecked();
    bool co2 = ui->co2CheckBox->isChecked();
    bool jura = ui->JuraCheckBox->isChecked();
    bool juraterm = ui->juraTermCheckBox->isChecked();
    bool motion = ui->motionCheckBox->isChecked();

    node.modules = 0x0;
    if (tnh) { node.modules |= MOD_TEMPERATURE_HUMIDITY; }
    if (pwm) { node.modules |= MOD_PWM; }
    if (io) { node.modules |= MOD_IO; }
    if (sw) { node.modules |= MOD_SWITCH; }
    if (co2) { node.modules |= MOD_CO2; }
    if (jura) { node.modules |= MOD_JURA; }
    if (juraterm) { node.modules |= MOD_JURATERM; }
    if (motion) { node.modules |= MOD_MOTION; }
    
    string flags = string(((char*) &node.modules), 4);

    // Publish the updated configuration.
    mqtt->publishMessage("cc/" + node.uid, "loc;" + node.location);
    mqtt->publishMessage("cc/" + node.uid, "mod;" + flags);
}


// --- NODE SELECTED ---
// Slot called when a node is selected in the UI. The node pointer is used to connect signals.
void MainWindow::nodeSelected(NodeTextItem* nodeTextItem) {
    if (nodeItem) {
        // disconnect everything on the current item before connecting the new item.
        //QObject::disconnect(nodeItem, 0, 0, 0);
        cout << "Disconnecting signals...\n";
        this->disconnect(nodeItem);
    }
    
    // Read out node information and set it in the UI.
    Node node = nodeTextItem->nodeData();
    savedNode = node.saved; // Does this node exist on the C&C server?
    ui->NodeUidEdit->setText(QString::fromStdString(node.uid));
    ui->nodeLocationEdit->setText(QString::fromStdString(node.location));
    ui->posxSpinBox->setValue(node.posx);
    ui->posySpinBox->setValue(node.posy);
    
    cout << "Modules: 0x" << std::hex << node.modules << "\n";
    if (node.modules & MOD_TEMPERATURE_HUMIDITY) {
        ui->tempHumidityCheckBox->setChecked(true);
    }
    else {
        ui->tempHumidityCheckBox->setChecked(false);
    }
    
    if (node.modules & MOD_MOTION) {
        ui->motionCheckBox->setChecked(true);
    }
    else {
        ui->motionCheckBox->setChecked(false);
    }

    if (node.modules & MOD_PWM) {
        ui->pwmCheckBox->setChecked(true);
    }
    else {
        ui->pwmCheckBox->setChecked(false);
    }

    if (node.modules & MOD_IO) {
        ui->ioCheckBox->setChecked(true);
    }
    else {
        ui->ioCheckBox->setChecked(false);
    }

    if (node.modules & MOD_SWITCH) {
        ui->switchCheckBox->setChecked(true);
    }
    else {
        ui->switchCheckBox->setChecked(false);
    }
    
    if (node.modules == 0) {
        ui->noUartCheckBox->setChecked(true);
    }
    else {
        
        if (node.modules & MOD_CO2) {
            ui->co2CheckBox->setChecked(true);
        }
        else {
            ui->co2CheckBox->setChecked(false);
        }
        
        if (node.modules & MOD_JURA) {
            ui->JuraCheckBox->setChecked(true);
        }
        else {
            ui->JuraCheckBox->setChecked(false);
        }
        
        if (node.modules & MOD_JURATERM) {
            ui->juraTermCheckBox->setChecked(true);
        }
        else {
            ui->juraTermCheckBox->setChecked(false);
        }
    }
    
    // Connect new signals.
    nodeItem = nodeTextItem;
    connect(this, SIGNAL(uidChanged(QString)), nodeItem, SLOT(updateUid(QString)));
    connect(this, SIGNAL(locationChanged(QString)), nodeItem, SLOT(updateLocation(QString)));
    connect(this, SIGNAL(positionXChanged(float)), nodeItem, SLOT(updatePosX(float)));
    connect(this, SIGNAL(positionYChanged(float)), nodeItem, SLOT(updatePosY(float)));
    connect(this, SIGNAL(modulesChanged(quint32)), nodeItem, SLOT(updateModules(quint32)));
    connect(nodeItem, SIGNAL(positionChanged(float,float)), this, SLOT(itemMoved(float,float)));
}


// --- SERIALIZE NODE ---
// Serialises the Node information currently being displayed and returns a string with the
// serialised info.
bool MainWindow::serializeNode(string &data) {
    QString uid = ui->NodeUidEdit->text();
    if (uid.isEmpty()) { return false; }
    
    Node node;
    node.uid = ui->NodeUidEdit->text().toStdString();
    node.location = ui->nodeLocationEdit->text().toStdString();
    node.posx = ui->posxSpinBox->value();
    node.posy = ui->posySpinBox->value();
    
    bool tnh = ui->tempHumidityCheckBox->isChecked();
    bool pwm = ui->pwmCheckBox->isChecked();
    bool io = ui->ioCheckBox->isChecked();
    bool sw = ui->switchCheckBox->isChecked();
    bool co2 = ui->co2CheckBox->isChecked();
    bool jura = ui->JuraCheckBox->isChecked();
    bool juraterm = ui->juraTermCheckBox->isChecked();
    bool motion = ui->motionCheckBox->isChecked();
    
    node.modules = 0x0;
    if (tnh) { node.modules |= MOD_TEMPERATURE_HUMIDITY; }
    if (pwm) { node.modules |= MOD_PWM; }
    if (io) { node.modules |= MOD_IO; }
    if (sw) { node.modules |= MOD_SWITCH; }
    if (co2) { node.modules |= MOD_CO2; }
    if (jura) { node.modules |= MOD_JURA; }
    if (juraterm) { node.modules |= MOD_JURATERM; }
    if (motion) { node.modules |= MOD_MOTION; }
    
    // Serialise the Node instance.
    string nodeStr = "NODE";
    quint8 length = (quint8) node.uid.length();
    nodeStr += string((char*) &length, 1);
    nodeStr += node.uid;
    length = (quint8) node.location.length();
    nodeStr += string((char*) &length, 1);
    nodeStr += node.location;
    nodeStr += string((char*) &node.posx, 4);
    nodeStr += string((char*) &node.posy, 4);
    nodeStr += string((char*) &node.modules, 4);
    quint32 segSize = nodeStr.length();
    
    data = string((char*) &segSize, 4);
    data += nodeStr;
    
    return true;
}


// --- SAVE NODE ---
// Save the currently displayed Node data to the C&C server.
void MainWindow::saveNode() {
    if (!connected) {
        QMessageBox::warning(this, tr("Not connected"), tr("Not connected to MQTT server."));
        return;
    }
    
    string message;
    if (!serializeNode(message)) { return; } // No selected node.
    if (!savedNode) { mqtt->publishMessage("cc/nodes/new", message); savedNode = true; }
    else { mqtt->publishMessage("cc/nodes/update", message); }
}


// --- SET NODES ---
void MainWindow::setNodes(QVector<Node> nodes) {
    cout << "Received list of new nodes...\n";
    
    // Clear the scene, then set each node instance in the vector in the UI.
    ui->graphicsView->scene()->clear();
    
    for (int i = 0; i < nodes.length(); ++i) {
        NodeTextItem* object = new NodeTextItem;
        cout << "Adding node for UID: " << nodes[i].uid << "\n";
        object->setNodeData(nodes[i]);
        object->setText(QString::fromStdString(nodes[i].location));
        object->setPos(nodes[i].posx, nodes[i].posy);
        object->setFont(QFont("Arial", 16, QFont::Bold));
        object->setPen(QPen(QColor("darkslateblue"), 1.5, Qt::SolidLine, Qt::RoundCap,
                            Qt::RoundJoin));
        
        QLinearGradient gradient(0, 0, 0, 100);
        gradient.setColorAt(0.0, "mediumslateblue");
        gradient.setColorAt(1.0, "cornsilk");
        object->setBrush(gradient);
        object->setFlags(QGraphicsItem::ItemIsSelectable | 
                         QGraphicsItem::ItemIsMovable |
                         QGraphicsItem::ItemSendsGeometryChanges);
        ui->graphicsView->scene()->addItem(object);
        
        // Connect the object to respond when it's being selected.
        connect(object, SIGNAL(nodeSelected(NodeTextItem*)), this, SLOT(nodeSelected(NodeTextItem*)));
    }
}


// --- SET MAP ---
// Change the background and graphics view's maximum size using the provided
// image data.
void MainWindow::setMap(QPixmap map) {
    cout << "Received new background image...\n";
    
    // Set background of scene.
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setSceneRect(0, 0, map.width(), map.height());
    ui->graphicsView->setBackgroundBrush(map);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    ui->graphicsView->show();
    
    // Request node data.
    updateNodes();
}


// --- DELETE NODE ---
// Check whether a node has been selected, then confirm deleting it.
// After sending the MQTT command, clear the removed node from the UI.
void MainWindow::deleteNode() {
    QString uid = ui->NodeUidEdit->text();
    if (uid.isEmpty()) { return; }
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm delete"), tr("Really delete this node?"),
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // Delete the node.
        string message = uid.toStdString();
        mqtt->publishMessage("cc/nodes/delete", message);
        
        // Clear the UI.
        ui->NodeUidEdit->clear();
        ui->nodeLocationEdit->clear();
        ui->tempHumidityCheckBox->setChecked(false);
        ui->pwmCheckBox->setChecked(false);
        ui->ioCheckBox->setChecked(false);
        ui->switchCheckBox->setChecked(false);
        ui->co2CheckBox->setChecked(false);
        ui->JuraCheckBox->setChecked(false);
        ui->juraTermCheckBox->setChecked(false);
        ui->co2EventsCheckBox->setChecked(false);
        ui->motionCheckBox->setChecked(false);
        
        // Delete the node in the UI.
        delete nodeItem;
        nodeItem = 0;
    }
}


// --- ERROR HANDLER ---
void MainWindow::errorHandler(QString err) {
    QMessageBox::warning(this, tr("Error"), err);
    connected = false;
}


// --- REGISTER NODE ---
// Allows the user to add a new node to the system.
// Information we need from the user to create a new node:
// * UID
// * location
void MainWindow::registerNode() {
    if (!connected) {
        QMessageBox::warning(this, tr("Not connected"), tr("Not connected to MQTT server."));
        return;
    }

    Node node;
    node.uid = QInputDialog::getText(this, tr("New node UID"), 
                                     tr("Unique ID for node")).toStdString();
    if (node.uid.empty()) { return; }
    node.location = QInputDialog::getText(this, tr("New node location"), 
                                          tr("Custom location string")).toStdString();
    if (node.location.empty()) { return; }
    
    // Fill in defaults.
    node.modules = 0x0;
    node.posx = 0.0;
    node.posy = 0.0;
    node.saved = false;
    
    // Set new node in UI.
    NodeTextItem* object = new NodeTextItem;
    object->setNodeData(node);
    object->setText(QString::fromStdString(node.location));
    object->setFont(QFont("Arial", 16, QFont::Bold));
    object->setPen(QPen(QColor("darkslateblue"), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    
    QLinearGradient gradient(0, 0, 0, 100);
    gradient.setColorAt(0.0, "mediumslateblue");
    gradient.setColorAt(1.0, "cornsilk");
    object->setBrush(gradient);
    object->setFlags(QGraphicsItem::ItemIsSelectable | 
                     QGraphicsItem::ItemIsMovable |
                     QGraphicsItem::ItemSendsGeometryChanges);
    ui->graphicsView->scene()->addItem(object);
    
    // Connect the object to respond when it's being selected.
    connect(object, SIGNAL(nodeSelected(NodeTextItem*)), this, SLOT(nodeSelected(NodeTextItem*)));
    
    // Publish.
    // TODO: Probably best handled by the saveNode() method.
    //mqtt->publishMessage("cc/nodes/new", "TODO");
}


// --- CONNECT REMOTE ---
// Connect to a remote C&C server.
void MainWindow::connectRemote() {
    if (connected) { return; }
    
    QString remote = QInputDialog::getText(this, tr("Remote MQTT broker"), 
                                           tr("Server URL or IP"),
                                           QLineEdit::Normal, remoteServer);
    if (remote.isEmpty()) { return; }
    
    int port = QInputDialog::getInt(this, tr("Remote broker port"), 
                                    tr("Server port"), remotePort);
    if (remotePort != port || remoteServer != remote) {
        remotePort = port;
        remoteServer = remote;
        QSettings settings;
        settings.setValue("lastRemote", remoteServer);
        settings.setValue("lastPort", remotePort);
    }
    
    // Create MQTT listener and set TLS options if appropriate.
    cout << "Initialising Libmosquitto library...\n";
    mosqpp::lib_init();
    mqtt = new MqttListener(0, "CnC-UI", remoteServer, remotePort);
    
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
                                                     tr("Connect securily"),
                                                     tr("Connect using TLS?"),
                                                     QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // Set TLS options.
        QString caFile = QInputDialog::getText(this, tr("CA path"), 
                                               tr("Full path to CA file."),
                                               QLineEdit::Normal, 
                                               QString::fromStdString(ca));
        QString certFile = QInputDialog::getText(this, tr("Cert path"), 
                                                 tr("Full path to Cert file."),
                                                 QLineEdit::Normal, 
                                                 QString::fromStdString(cert));
        QString keyFile = QInputDialog::getText(this, tr("Key path"), 
                                                tr("Full path to Key file."),
                                                QLineEdit::Normal, 
                                                QString::fromStdString(key));
        if (caFile.isEmpty() || certFile.isEmpty() || keyFile.isEmpty()) {
            // Abort connecting.
            return;
        }
        
        QSettings settings;
        settings.setValue("caFile", caFile);
        settings.setValue("certFile", certFile);
        settings.setValue("keyFile", keyFile);
        ca = caFile.toStdString();
        cert = certFile.toStdString();
        key = keyFile.toStdString();
        
        if (!mqtt->setTLS(ca, cert, key)) {
            cerr << "SetTLS failed.\n";
            QMessageBox::critical(this, tr("Setting TLS failed"), 
                                  tr("Setting the TLS parameters failed.\n\n\
                               Please check the provided info and try again."));
            
            return;
        }
    }
    
    // Try to connect. If successful, update local copy and stored remote server address.
    // The MQTT client class has to run on its own thread since it uses its own event loop.
    QThread* thread = new QThread();
    mqtt->moveToThread(thread);
    
    connect(mqtt, SIGNAL(connected()), this, SLOT(updateMap()));
    connect(mqtt, SIGNAL(failed(QString)), this, SLOT(errorHandler(QString)));
    connect(mqtt, SIGNAL(receivedMap(QPixmap)), this, SLOT(setMap(QPixmap)));
    connect(mqtt, SIGNAL(receivedNodes(QVector<Node>)), this, SLOT(setNodes(QVector<Node>)));
    connect(thread, SIGNAL(started()), mqtt, SLOT(connectBroker()));
    connect(mqtt, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), mqtt, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}


// --- SAVE CONFIGURATION ---
// Saves the node positions and configuration to the remote C&C server.
void MainWindow::saveConfiguration() {
    if (!connected) {
        QMessageBox::warning(this, tr("Not connected"), tr("Not connected to MQTT server."));
        return;
    }
    
    // TODO: implement.
}


// --- SHOW FIRMWARE DIALOGUE ---
void MainWindow::showFirmwareDialogue() {
    FirmwareDialogue* dialogue = new FirmwareDialogue(this);
    connect(dialogue, SIGNAL(newMessage(string,string)), this, SLOT(sendMessage(string,string)));
    connect(mqtt, SIGNAL(receivedFirmwareList(QString)), dialogue, SLOT(updateList(QString)));
    dialogue->exec();
}


// --- SHOW NODE FIRMWARE DIALOGUE ---
void MainWindow::showNodeFirmwareDialogue() {
    NodeFirmwareDialogue* dialogue = new NodeFirmwareDialogue(this);
    dialogue->exec();
}


// --- SEND MESSAGE ---
void MainWindow::sendMessage(string topic, string message) {
    if (!mqtt) { return; }
    
    mqtt->publishMessage(topic, message);
}


// --- QUIT ---
// Exits the application.
void MainWindow::quit() {
    exit(0);
}


// --- UID CHANGED --
void MainWindow::uidChanged() {
    QString uid = ui->NodeUidEdit->text();
    cout << "Changing uid to: " << uid.toStdString() << "\n";
    emit uidChanged(uid);
}


// --- LOCATION CHANGED ---
void MainWindow::locationChanged() {
    QString location = ui->nodeLocationEdit->text();
    cout << "Changing location to: " << location.toStdString() << "\n";
    emit locationChanged(location);
}


// --- POSITION X CHANGED ---
void MainWindow::positionXChanged(double value) {
    emit positionXChanged((float) value);
}


// --- POSITION Y CHANGED ---
void MainWindow::positionYChanged(double value) {
    emit positionYChanged((float) value);
}


// --- MODULES CHANGED ---
void MainWindow::modulesChanged(int state) {
    // Read out all checkbox states, create uint32 value and emit the signal.
    quint32 modules = 0x0;
    bool tnh = ui->tempHumidityCheckBox->isChecked();
    bool pwm = ui->pwmCheckBox->isChecked();
    bool io = ui->ioCheckBox->isChecked();
    bool sw = ui->switchCheckBox->isChecked();
    bool co2 = ui->co2CheckBox->isChecked();
    bool jura = ui->JuraCheckBox->isChecked();
    bool juraterm = ui->juraTermCheckBox->isChecked();
    bool motion = ui->motionCheckBox->isChecked();
    
    if (tnh) { modules |= MOD_TEMPERATURE_HUMIDITY; }
    if (pwm) { modules |= MOD_PWM; }
    if (io) { modules |= MOD_IO; }
    if (sw) { modules |= MOD_SWITCH; }
    if (co2) { modules |= MOD_CO2; }
    if (jura) { modules |= MOD_JURA; }
    if (juraterm) { modules |= MOD_JURATERM; }
    if (motion) { modules |= MOD_MOTION; }
    
    emit modulesChanged(modules);
}


// --- ITEM MOVED ---
void MainWindow::itemMoved(float posx, float posy) {
    //cout << "Item moved: " << posx << ", " << posy << "\n";
    ui->posxSpinBox->setValue(posx);
    ui->posySpinBox->setValue(posy);
}
