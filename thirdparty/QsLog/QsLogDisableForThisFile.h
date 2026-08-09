#ifndef QSLOGDISABLEFORTHISFILE_H
#define QSLOGDISABLEFORTHISFILE_H

#include <QtDebug>
// When included AFTER QsLog.h, this file will disable logging in that C++ file. When included
// before, it will lead to compiler warnings or errors about macro redefinitions.
// 在 QsLog.h 之后包含本文件时，将禁用该 C++ 文件中的日志记录；若在其之前包含，
// 则会因宏重复定义而产生编译器警告或错误。

#undef QLOG_TRACE
#undef QLOG_DEBUG
#undef QLOG_INFO
#undef QLOG_WARN
#undef QLOG_ERROR
#undef QLOG_FATAL

#define QLOG_TRACE() if (1) {} else qDebug()
#define QLOG_DEBUG() if (1) {} else qDebug()
#define QLOG_INFO()  if (1) {} else qDebug()
#define QLOG_WARN()  if (1) {} else qDebug()
#define QLOG_ERROR() if (1) {} else qDebug()
#define QLOG_FATAL() if (1) {} else qDebug()

#endif // QSLOGDISABLEFORTHISFILE_H
