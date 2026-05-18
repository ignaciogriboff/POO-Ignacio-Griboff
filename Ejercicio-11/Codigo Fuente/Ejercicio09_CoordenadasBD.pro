QT += core gui widgets sql

CONFIG += c++17

TEMPLATE = app
TARGET = Ejercicio09_CoordenadasBD

SOURCES += \
    src/main.cpp \
    src/database.cpp \
    src/logindialog.cpp \
    src/mainwindow.cpp \
    src/pintura.cpp

HEADERS += \
    src/database.h \
    src/logindialog.h \
    src/mainwindow.h \
    src/pintura.h

FORMS += \
    src/logindialog.ui

RESOURCES += \
    resources/resources.qrc
