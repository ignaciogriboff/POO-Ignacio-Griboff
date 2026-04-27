QT += widgets network websockets
CONFIG += c++17

TEMPLATE = app
TARGET = lienzo_colaborativo

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    canvaswidget.cpp \
    drawingmodel.cpp \
    realtimesyncservice.cpp \
    syncservice.cpp

HEADERS += \
    mainwindow.h \
    canvaswidget.h \
    drawingmodel.h \
    realtimesyncservice.h \
    syncservice.h