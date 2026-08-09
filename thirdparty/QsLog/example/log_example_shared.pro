#This project file links with QsLog dynamically and outputs a shared object.
#此项目文件动态链接 QsLog 并生成共享对象。

QT -= gui

TARGET = log_example_shared
TEMPLATE = lib

DEFINES += EXAMPLE_IS_SHARED_LIBRARY QSLOG_IS_SHARED_LIBRARY_IMPORT
SOURCES += log_example_shared.cpp
HEADERS += log_example_shared.h
INCLUDEPATH += $$PWD/../
DESTDIR = $$PWD/../build-QsLogExample
OBJECTS_DIR = $$DESTDIR/obj

LIBS += -L$$PWD/../build-QsLogShared
win32 {
    LIBS += -lQsLog2
} else {
    LIBS += -lQsLog
}


