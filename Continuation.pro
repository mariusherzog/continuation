TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

QMAKE_CXXFLAGS += -pthread -std=c++14


LIBS += -lpthread

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    continuation.h

