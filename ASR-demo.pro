QT       += core gui multimedia multimediawidgets x11extras KWindowSystem dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 \
        link_pkgconfig

PKGCONFIG += gsettings-qt

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lpthread -lX11 \
        -L/usr/lib/libukui-log4qt.so.1.0.0 -lukui-log4qt

target.path = /usr/bin
target.source += $$TARGET

# gsettings
schemas.files += \
        $$PWD/data/org.asr-demo.gschema.xml
schemas.path = /usr/share/glib-2.0/schemas/

INSTALLS += target schemas

SOURCES += \
    asrthread.cpp \
    fileoperation.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    asrthread.h \
    fileoperation.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    data/org.asr-demo.gschema.xml
