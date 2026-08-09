INCLUDEPATH += $$PWD
#DEFINES += QS_LOG_LINE_NUMBERS    # automatically writes the file and line for each log message
# 自动为每条日志消息写入文件名和行号。
#DEFINES += QS_LOG_DISABLE         # logging code is replaced with a no-op
# 将日志代码替换为空操作。
#DEFINES += QS_LOG_SEPARATE_THREAD # messages are queued and written from a separate thread
# 消息先进入队列，再由独立线程写出。
SOURCES += $$PWD/QsLogDest.cpp \
    $$PWD/QsLog.cpp \
    $$PWD/QsLogDestConsole.cpp \
    $$PWD/QsLogDestFile.cpp \
    $$PWD/QsLogDestFunctor.cpp

HEADERS += $$PWD/QsLogDest.h \
    $$PWD/QsLog.h \
    $$PWD/QsLogDestConsole.h \
    $$PWD/QsLogLevel.h \
    $$PWD/QsLogDestFile.h \
    $$PWD/QsLogDisableForThisFile.h \
    $$PWD/QsLogDestFunctor.h

OTHER_FILES += \
    $$PWD/QsLogChanges.txt \
    $$PWD/QsLogReadme.txt \
    $$PWD/LICENSE.txt
