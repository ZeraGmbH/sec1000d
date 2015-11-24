TEMPLATE	= app
LANGUAGE	= C++

include(sec1000d.user.pri)

QMAKE_CXXFLAGS += -O0

systemd_notification {
       DEFINES += SYSTEMD_NOTIFICATION
       LIBS +=  -lsystemd
}

LIBS +=  -lSCPI
LIBS +=  -lproto-net-qt
LIBS +=  -lzeramisc
LIBS +=  -lzeraxmlconfig
LIBS +=  -lzeramath
LIBS +=  -lprotobuf
LIBS +=  -lzera-resourcemanager-protobuf

CONFIG	+= qt debug

HEADERS	+= \
    zeraglobal.h \
    sec1000d.h \
    sec1000dscpi.h \
    pcbserver.h \
    sec1000dglobal.h \
    ethsettings.h \
    fpgasettings.h \
    scpidelegate.h \
    statusinterface.h \
    scpiconnection.h \
    systeminterface.h \
    xmlsettings.h \
    debugsettings.h \
    resource.h \
    systeminfo.h \
    rmconnection.h \
    sec1000dprotobufwrapper.h \
    notificationdata.h \
    protonetcommand.h \
    ecalcsettings.h \
    ecalcinterface.h \
    ecalcchannel.h \
    notificationvalue.h

SOURCES	+= \
    main.cpp \
    sec1000d.cpp \
    pcbserver.cpp \
    ethsettings.cpp \
    fpgasettings.cpp \
    scpidelegate.cpp \
    statusinterface.cpp \
    scpiconnection.cpp \
    systeminterface.cpp \
    debugsettings.cpp \
    systeminfo.cpp \
    resource.cpp \
    rmconnection.cpp \
    sec1000dprotobufwrapper.cpp \
    protonetcommand.cpp \
    ecalcsettings.cpp \
    ecalcinterface.cpp \
    ecalcchannel.cpp \
    notificationvalue.cpp

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

target.path = /usr/bin
INSTALLS += target

configxml.path = /etc/zera/sec1000d
configxml.files = sec1000d.xsd \
                  sec1000d.xml

INSTALLS += configxml

QT += xml network

OTHER_FILES +=
