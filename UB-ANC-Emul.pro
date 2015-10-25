#-------------------------------------------------
#
# Project created by QtCreator 2015-10-16T20:53:48
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = emulator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#
# APM Planner Library
#
include (apm_planner.pri)

INCLUDEPATH += \
    engine \

HEADERS += \
    engine/UBEngine.h \
    engine/UBObject.h \
    engine/UBServer.h \
    config.h \

SOURCES += \
    engine/UBEngine.cpp \
    engine/UBObject.cpp \
    engine/UBServer.cpp \
