QT += core gui widgets

CONFIG += c++17

TEMPLATE = app
TARGET = Ejercicio08_EditorMultilenguaje

SOURCES += \
    src/main.cpp \
    src/appconfig.cpp \
    src/logger.cpp \
    src/pantalla.cpp \
    src/validadores.cpp \
    src/login_screen.cpp \
    src/blocked_screen.cpp \
    src/editor_screen.cpp \
    src/application_controller.cpp

HEADERS += \
    src/appconfig.h \
    src/logger.h \
    src/pantalla.h \
    src/validadores.h \
    src/login_screen.h \
    src/blocked_screen.h \
    src/editor_screen.h \
    src/application_controller.h
