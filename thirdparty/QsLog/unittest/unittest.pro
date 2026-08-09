QT += core

TARGET = QsLogUnitTest
CONFIG += console qtestlib
CONFIG -= app_bundle
TEMPLATE = app

# test-case sources
# 测试用例源文件
SOURCES += TestLog.cpp

# component sources
# 组件源文件
include(../QsLog.pri)

SOURCES += \
    ./QtTestUtil/TestRegistry.cpp \
    ./QtTestUtil/SimpleChecker.cpp

HEADERS += \
    ./QtTestUtil/TestRegistry.h \
    ./QtTestUtil/TestRegistration.h \
    ./QtTestUtil/QtTestUtil.h
