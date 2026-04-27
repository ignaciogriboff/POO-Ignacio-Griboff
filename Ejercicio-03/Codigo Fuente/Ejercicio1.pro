QT += widgets
CONFIG += c++17

SOURCES += \
    main.cpp \
    loginwidget.cpp \
    userstore.cpp \
    sessionmanager.cpp \
    tpitem.cpp \
    mainwindowwidget.cpp \
    tpeditdialog.cpp \
    notesstore.cpp \
    notesdialog.cpp \
    historylogger.cpp

HEADERS += \
    loginwidget.h \
    userstore.h \
    sessionmanager.h \
    tpitem.h \
    mainwindowwidget.h \
    tpeditdialog.h \
    notesstore.h \
    notesdialog.h \
    historylogger.h

usersjson.files = $$PWD/users.json
usersjson.path = $$OUT_PWD
INSTALLS += usersjson

DISTFILES += \
    .gitignore \
    users.json