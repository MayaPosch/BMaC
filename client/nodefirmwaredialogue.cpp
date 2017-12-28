#include "nodefirmwaredialogue.h"
#include "ui_nodefirmwaredialogue.h"

NodeFirmwareDialogue::NodeFirmwareDialogue(QWidget *parent) :
        QDialog(parent), ui(new Ui::NodeFirmwareDialogue) {
    ui->setupUi(this);
    
    // Connections.
    connect(this, SIGNAL(accepted()), this, SLOT(slotOk()));
    connect(this, SIGNAL(rejected()), this, SLOT(slotCancel()));
}

NodeFirmwareDialogue::~NodeFirmwareDialogue() {
    delete ui;
}


// --- SLOT OK ---
void NodeFirmwareDialogue::slotOk() {
    // Submit changed nodes to the C&C server.
    
    setResult(QDialog::Accepted);
    close();
}


// --- SLOT CANCEL ---
void NodeFirmwareDialogue::slotCancel() {
    setResult(QDialog::Rejected);
    close();
}
