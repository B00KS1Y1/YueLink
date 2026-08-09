qt_add_library(QsLog STATIC
    QsLog.cpp
    QsLog.h
    QsLogDest.cpp
    QsLogDest.h
    QsLogDestConsole.cpp
    QsLogDestConsole.h
    QsLogDestFile.cpp
    QsLogDestFile.h
    QsLogDestFunctor.cpp
    QsLogDestFunctor.h
    QsLogDisableForThisFile.h
    QsLogLevel.h
)

add_library(QsLog::QsLog ALIAS QsLog)

option(QSLOG_ENABLE_LINE_NUMBERS "Include source locations in QsLog messages" OFF)
option(QSLOG_DISABLE_LOGGING "Compile out all QsLog messages" OFF)
option(QSLOG_USE_SEPARATE_THREAD "Write QsLog messages from a separate thread" OFF)

target_include_directories(QsLog
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(QsLog
    PUBLIC
        $<$<BOOL:${QSLOG_ENABLE_LINE_NUMBERS}>:QS_LOG_LINE_NUMBERS>
        $<$<BOOL:${QSLOG_DISABLE_LOGGING}>:QS_LOG_DISABLE>
    PRIVATE
        $<$<BOOL:${QSLOG_USE_SEPARATE_THREAD}>:QS_LOG_SEPARATE_THREAD>
)

target_link_libraries(QsLog
    PUBLIC
        Qt6::Core
)
