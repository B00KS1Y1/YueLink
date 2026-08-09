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

option(QSLOG_DISABLE_LOGGING "Compile out all QsLog messages" OFF)

target_include_directories(QsLog
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(QsLog
    PUBLIC
        $<$<BOOL:${QSLOG_DISABLE_LOGGING}>:QS_LOG_DISABLE>
)

target_link_libraries(QsLog
    PUBLIC
        Qt6::Core
)
