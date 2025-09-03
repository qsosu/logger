QT       += core gui network sql xml
QT       += quickwidgets positioning qml quick
QT       += serialport printsupport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += sdk_no_version_check

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    about.cpp \
    addcallsign.cpp \
    apilogradio.cpp \
    callsigns.cpp \
    cat_interface.cpp \
    chatcontroller.cpp \
    confirmqso.cpp \
    exportadif.cpp \
    flowlayout.cpp \
    flrig.cpp \
    geolocation.cpp \
    httpapi.cpp \
    importadif.cpp \
    main.cpp \
    mainwindow.cpp \
    mapcontroller.cpp \
    qrzrucallbook.cpp \
    qsoedit.cpp \
    qsopanel.cpp \
    reports/reportbands.cpp \
    reports/reportcontinent.cpp \
    reports/reportcountry.cpp \
    reports/reportmodes.cpp \
    reports/reportsunchart.cpp \
    settings.cpp \
    spotviewer.cpp \
    telnetclient.cpp \
    thirdparty/libmaia/maia/maiaFault.cpp \
    thirdparty/libmaia/maia/maiaObject.cpp \
    thirdparty/libmaia/maia/maiaXmlRpcClient.cpp \
    thirdparty/libmaia/maia/maiaXmlRpcServer.cpp \
    thirdparty/libmaia/maia/maiaXmlRpcServerConnection.cpp \
    udpserver.cpp \
    updatelogprefix.cpp \
    uploadinglogs.cpp

HEADERS += \
    about.h \
    addcallsign.h \
    apilogradio.h \
    callsigns.h \
    cat_interface.h \
    chatcontroller.h \
    confirmqso.h \
    delegations.h \
    exportadif.h \
    flowlayout.h \
    flrig.h \
    geolocation.h \
    ham_definitions.h \
    helpers.h \
    httpapi.h \
    importadif.h \
    mainwindow.h \
    mapcontroller.h \
    qrzrucallbook.h \
    qsoedit.h \
    qsopanel.h \
    reports/reportbands.h \
    reports/reportcontinent.h \
    reports/reportcountry.h \
    reports/reportmodes.h \
    reports/reportsunchart.h \
    settings.h \
    spotviewer.h \
    telnetclient.h \
    thirdparty/libmaia/maia/maiaFault.h \
    thirdparty/libmaia/maia/maiaObject.h \
    thirdparty/libmaia/maia/maiaXmlRpcClient.h \
    thirdparty/libmaia/maia/maiaXmlRpcServer.h \
    thirdparty/libmaia/maia/maiaXmlRpcServerConnection.h \
    udpserver.h \
    updatelogprefix.h \
    uploadinglogs.h

FORMS += \
    about.ui \
    addcallsign.ui \
    callsigns.ui \
    chatcontroller.ui \
    confirmqso.ui \
    exportadif.ui \
    geolocation.ui \
    importadif.ui \
    mainwindow.ui \
    qsoedit.ui \
    qsopanel.ui \
    reports/reportbands.ui \
    reports/reportcontinent.ui \
    reports/reportcountry.ui \
    reports/reportmodes.ui \
    reports/reportsunchart.ui \
    settings.ui \
    spotviewer.ui \
    updatelogprefix.ui \
    uploadinglogs.ui

INCLUDEPATH += thirdparty/libmaia

win32:RC_ICONS = $$PWD/resources/images/icon32.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc \
    qml.qrc

TRANSLATIONS += en_US.ts

DISTFILES += \
    resources/images/bell.png \
    resources/images/bell_new.png \
    resources/images/notification.png \
    resources/images/notification_new.png



