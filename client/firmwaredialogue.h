#ifndef FIRMWAREDIALOGUE_H
#define FIRMWAREDIALOGUE_H

#include <QDialog>

#include <string>

using namespace std;


namespace Ui {
class FirmwareDialogue;
}

class FirmwareDialogue : public QDialog {
    Q_OBJECT
    
public:
    explicit FirmwareDialogue(QWidget *parent = 0);
    ~FirmwareDialogue();
    
private:
    Ui::FirmwareDialogue *ui;
    
private slots:    
    void slotOk();
    void uploadFirmware();
 
public slots:
    void updateList(QString list);
    
signals:
    void newMessage(string topic, string message);
};

#endif // FIRMWAREDIALOGUE_H
