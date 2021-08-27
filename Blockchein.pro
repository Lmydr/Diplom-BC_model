QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -L"C:\Program Files\OpenSSL\lib" -llibssl -llibcrypto
INCLUDEPATH += "C:\Program Files\OpenSSL\include"

SOURCES += \
    client.cpp \
    hashblockcreator.cpp \
    main.cpp \
    maincore.cpp \
    mainwindow.cpp \
    sendtokensform.cpp \
    wallet.cpp \
    walletform.cpp

HEADERS += \
    Structs.h \
    client.h \
    hashblockcreator.h \
    maincore.h \
    mainwindow.h \
    sendtokensform.h \
    wallet.h \
    walletform.h

FORMS += \
    mainwindow.ui \
    sendtokensform.ui \
    walletform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
