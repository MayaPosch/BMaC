#include "firmwaredialogue.h"
#include "ui_firmwaredialogue.h"

#include <QFileDialog>

FirmwareDialogue::FirmwareDialogue(QWidget *parent) :
        QDialog(parent), ui(new Ui::FirmwareDialogue) {
    ui->setupUi(this);
    
    // Connections.
    connect(ui->uploadButton, SIGNAL(pressed()), this, SLOT(uploadFirmware()));
    
    // Get new list of firmware files.
    string topic = "cc/firmware";
    string message = "list";
    emit newMessage(topic, message);
}

FirmwareDialogue::~FirmwareDialogue() {
    delete ui;
}


// --- UPDATE LIST ---
void FirmwareDialogue::updateList(QString list) {
    // Split the list using the ';' separators, then fill the list widget with
    // the names.
    QStringList listParts = list.split(';', QString::SkipEmptyParts);
    for (int i = 0; i < listParts.size(); ++i) {
        new QListWidgetItem(listParts[i], ui->listWidget);
    }
}


// --- SLOT OK ---
void FirmwareDialogue::slotOk() {
    accept();
}


// --- UPLOAD FIRMWARE ---
void FirmwareDialogue::uploadFirmware() {
    // Ask for path to firmware image, then read it into a string along with its
    // filename.
    QString filename = QFileDialog::getOpenFileName(this, tr("New firmware image"));
    if (filename.isEmpty()) { return; }
    
    //
}
