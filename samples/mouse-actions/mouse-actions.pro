TARGET = qgeoview-samples-mouse-actions
TEMPLATE = app
CONFIG-= console

QT += gui widgets network sql

include(../lib.pri)
include(../shared.pri)

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h
