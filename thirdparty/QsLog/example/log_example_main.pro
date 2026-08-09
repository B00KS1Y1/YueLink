#This project links with QsLog dynamically and outputs an executable file.
#此项目动态链接 QsLog 并生成可执行文件。

QT -= gui
TARGET = log_example
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += log_example_main.cpp
INCLUDEPATH += $$PWD/../
DEFINES += QSLOG_IS_SHARED_LIBRARY_IMPORT

LIBS += -L$$PWD/../build-QsLogShared
win32 {
    LIBS += -lQsLog2
} else {
    LIBS += -lQsLog
}
LIBS += -L$$PWD/../build-QsLogExample -llog_example_shared

DESTDIR = $$PWD/../build-QsLogExample
OBJECTS_DIR = $$DESTDIR/obj
