QT += widgets network

SOURCES += \
    app_config.cpp \
    logger.cpp \
    main.cpp \
    login.cpp \
    ventana.cpp \
    clima_service.cpp \
    image_cache_service.cpp

HEADERS += \
    app_config.h \
    logger.h \
    login.h \
    ventana.h \
    clima_service.h \
    image_cache_service.h

FORMS += \
    login.ui \
    ventana.ui

DISTFILES += \
    config.ini