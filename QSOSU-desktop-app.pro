QT       += core gui network sql xml
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    about.cpp \
    addcallsign.cpp \
    adif.cpp \
    callsigns.cpp \
    flrig.cpp \
    httpapi.cpp \
    loggercat.cpp \
    main.cpp \
    mainwindow.cpp \
    qrzrucallbook.cpp \
    settings.cpp \
    thirdparty/libmaia/maia/maiaFault.cpp \
    thirdparty/libmaia/maia/maiaObject.cpp \
    thirdparty/libmaia/maia/maiaXmlRpcClient.cpp \
    thirdparty/libmaia/maia/maiaXmlRpcServer.cpp \
    thirdparty/libmaia/maia/maiaXmlRpcServerConnection.cpp \
    udpreceiver.cpp

HEADERS += \
    about.h \
    addcallsign.h \
    adif.h \
    callsigns.h \
    delegations.h \
    flrig.h \
    helpers.h \
    httpapi.h \
    loggercat.h \
    mainwindow.h \
    qrzrucallbook.h \
    settings.h \
    thirdparty/libmaia/maia/maiaFault.h \
    thirdparty/libmaia/maia/maiaObject.h \
    thirdparty/libmaia/maia/maiaXmlRpcClient.h \
    thirdparty/libmaia/maia/maiaXmlRpcServer.h \
    thirdparty/libmaia/maia/maiaXmlRpcServerConnection.h \
    udpreceiver.h

FORMS += \
    about.ui \
    addcallsign.ui \
    callsigns.ui \
    loggercat.ui \
    mainwindow.ui \
    settings.ui

INCLUDEPATH += thirdparty/libmaia

win32:RC_ICONS = $$PWD/resources/images/icon32.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

