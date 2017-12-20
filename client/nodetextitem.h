#ifndef NODETEXTITEM_H
#define NODETEXTITEM_H


#include "mqttlistener.h"

#include <QGraphicsSimpleTextItem>
#include <QMutex>


class NodeTextItem :  public QObject, public QGraphicsSimpleTextItem {
    Q_OBJECT
    
    Node node;
    QMutex mutex;
    
public:
    NodeTextItem();
    
    void setNodeData(Node node);
    Node nodeData();
    
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    
signals:
    void nodeSelected(NodeTextItem* node);
    void positionChanged(float posx, float posy);
    
public slots:
    void updateUid(QString uid);
    void updateLocation(QString location);
    void updateModules(quint32 modules);
    void updatePosX(float posx);
    void updatePosY(float posy);
};

#endif // NODETEXTITEM_H
