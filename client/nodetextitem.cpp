#include "nodetextitem.h"

#include <iostream>

using namespace std;


NodeTextItem::NodeTextItem() {
    //
}


// --- SET NODE DATA ---
void NodeTextItem::setNodeData(Node node) {
    this->node = node;
}


// --- NODE DATA ---
Node NodeTextItem::nodeData() {
    Node returnNode;
    mutex.lock();
    returnNode = this->node;
    mutex.unlock();
    return returnNode;
}


// --- ITEM CHANGE ---
QVariant NodeTextItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    // Respond to changes in the item's position and selection state.
    // * Update the internal X/Y position when the item has been moved.
    // * Emit the nodeSelected signal.
    switch (change) {
    case QGraphicsItem::ItemPositionHasChanged: {
            QPointF pos = this->pos();
            //mutex.lock();
            //node.posx = pos.x();
            //node.posy = pos.y();
            //mutex.unlock();
            emit positionChanged(pos.x(), pos.y());
            break;
        }
    case QGraphicsItem::ItemSelectedHasChanged: {
            if (this->isSelected()) {
                emit nodeSelected((NodeTextItem*) this);
            }
            
            break;
        }
    }
    
    return QGraphicsItem::itemChange(change, value);
}


// --- UPDATE UID ---
void NodeTextItem::updateUid(QString uid) {
    mutex.lock();
    node.uid = uid.toStdString();
    mutex.unlock();
}


// --- UPDATE LOCATION ---
void NodeTextItem::updateLocation(QString location) {
    mutex.lock();
    node.location = location.toStdString();
    setText(location);
    mutex.unlock();
}


// --- UPDATE MODULES ---
void NodeTextItem::updateModules(quint32 modules){
    mutex.lock();    
    cout << "Updating modules to: 0x" << std::hex << modules << "\n";
    node.modules = modules;
    mutex.unlock();
}


// --- UPDATE POS X ---
void NodeTextItem::updatePosX(float posx) {
    mutex.lock();
    node.posx = posx;
    mutex.unlock();
}


// --- UPDATE POS Y ---
void NodeTextItem::updatePosY(float posy) {
    mutex.lock();
    node.posy = posy;
    mutex.unlock();
}
