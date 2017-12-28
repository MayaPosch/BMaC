#ifndef NODEFIRMWAREDIALOGUE_H
#define NODEFIRMWAREDIALOGUE_H

#include <QDialog>

namespace Ui {
class NodeFirmwareDialogue;
}

class NodeFirmwareDialogue : public QDialog {
    Q_OBJECT
    
public:
    explicit NodeFirmwareDialogue(QWidget *parent = 0);
    ~NodeFirmwareDialogue();
    
private:   
    Ui::NodeFirmwareDialogue *ui;
    
private slots:    
    void slotOk();
    void slotCancel();
};

#endif // NODEFIRMWAREDIALOGUE_H
