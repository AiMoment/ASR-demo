QT       += core gui multimedia websockets dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# For baidu ai
DEFINES += WEBSOCKET_AI

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ai/iflytek_utils.cpp \
    ai/kvp_aiengine.cpp \
    ai/websocketclientmanager.cpp \
    dbus/kvp_dbusadapter.cpp \
    input/kvp_input.cpp \
    input/kvp_input_file.cpp \
    input/kvp_input_real.cpp \
    kvp_actions.cpp \
    kvp_appfunctions.cpp \
    kvp_resample.cpp \
    main.cpp \
    packet/kvp_packet.cpp \
    voice/kvp_voicetransfer.cpp \
    widget.cpp

HEADERS += \
    ai/iflytek_utils.h \
    ai/kvp_aiengine.h \
    ai/websocketclientmanager.h \
    dbus/kvp_dbusadapter.h \
    input/kvp_input.h \
    input/kvp_input_file.h \
    input/kvp_input_real.h \
    kvp_actions.h \
    kvp_appfunctions.h \
    kvp_resample.h \
    packet/kvp_packet.h \
    voice/kvp_voicetransfer.h \
    widget.h

FORMS += \
    widget.ui

LIBS += -lswresample -lavutil -lboost_system -lpthread -lcrypto -lssl

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

