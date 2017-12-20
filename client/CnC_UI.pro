#-------------------------------------------------
#
# Project created by QtCreator 2017-03-27T17:58:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CnC_UI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mqttlistener.cpp \
    nodetextitem.cpp

HEADERS  += mainwindow.h \
    mqttlistener.h \
    nodetextitem.h

FORMS    += mainwindow.ui

DISTFILES +=

RESOURCES +=

LIBS += -lmosquittopp -lmosquitto -L/usr/local/lib

INCLUDEPATH += /usr/local/include

win32: LIBS += -L$$PWD/mosquitto/ -lmosquitto

INCLUDEPATH += $$PWD/mosquitto
DEPENDPATH += $$PWD/mosquitto

win32: LIBS += -L$$PWD/mosquitto/ -lmosquittopp

INCLUDEPATH += $$PWD/mosquitto
DEPENDPATH += $$PWD/mosquitto
