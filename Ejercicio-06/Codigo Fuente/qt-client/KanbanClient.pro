QT += core gui widgets network websockets

CONFIG += c++17

TARGET = KanbanClient
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    api_client.cpp \
    ws_client.cpp \
    dialogs/columndialog.cpp \
    dialogs/carddialog.cpp

HEADERS += \
    mainwindow.h \
    api_client.h \
    ws_client.h \
    models.h \
    dialogs/columndialog.h \
    dialogs/carddialog.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
