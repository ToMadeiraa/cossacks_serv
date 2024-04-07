QT       += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# TARGET = cossacks_serv
# TEMPLATE = app

CONFIG+=sdk_no_version_check

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
        main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

