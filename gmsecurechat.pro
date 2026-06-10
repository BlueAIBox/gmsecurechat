QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

TARGET = gmsecurechat
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sm2.cpp \
    sm3.cpp \
    sm4.cpp \
    zuc.cpp \
    udpmanager.cpp \
    messagehandler.cpp \
    filehandler.cpp \
    infomanager.cpp \
    logger.cpp \
    utils.cpp \
    securitydashboard.cpp \
    localdappserver.cpp \
    hermesbridge.cpp

HEADERS += \
    mainwindow.h \
    sm2.h \
    sm3.h \
    sm4.h \
    zuc.h \
    udpmanager.h \
    messagehandler.h \
    filehandler.h \
    infomanager.h \
    logger.h \
    utils.h \
    securitydashboard.h \
    localdappserver.h \
    hermesbridge.h

FORMS += mainwindow.ui

OPENSSL_ROOT = $$getenv(OPENSSL_ROOT_DIR)
isEmpty(OPENSSL_ROOT):exists(D:/OpenSSL-Win64/include/openssl/evp.h) {
    OPENSSL_ROOT = D:/OpenSSL-Win64
}
isEmpty(OPENSSL_ROOT) {
    OPENSSL_ROOT = $$[QT_INSTALL_PREFIX]
}

INCLUDEPATH += $$OPENSSL_ROOT/include
LIBS += -L$$OPENSSL_ROOT/lib -lcrypto -lssl
