#-------------------------------------------------
#
# Project created by QtCreator 2015-10-16T20:53:48
#
#-------------------------------------------------

QT       += core concurrent

QT       -= gui

TARGET = ub-anc-emulator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#
# APM Planner Library
#
include(apm_planner.pri)

#
# NS-3 Network Library
#
include(ns_3.pri)

INCLUDEPATH += \
    engine \
    network \

HEADERS += \
    engine/UBEngine.h \
    engine/UBObject.h \
    engine/UBServer.h \
    config.h \
    engine/NS3Engine.h \
    network/UBNetPacket.h

SOURCES += \
    engine/UBEngine.cpp \
    engine/UBObject.cpp \
    engine/UBServer.cpp \
    engine/NS3Engine.cpp \
    network/UBNetPacket.cpp
