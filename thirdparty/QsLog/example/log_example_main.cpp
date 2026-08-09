#include "QsLog.h"
#include "QsLogDest.h"
#include "log_example_shared.h"
#include <QLibrary>
#include <QCoreApplication>
#include <QDir>
#include <iostream>

void logFunction(const QString &message, QsLogging::Level level)
{
    std::cout << "From log function: " << qPrintable(message) << " " << static_cast<int>(level)
              << std::endl;
}

// This small example shows how QsLog can be used inside a project.
// 这个小示例展示了如何在项目中使用 QsLog。
int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);

   using namespace QsLogging;

   // 1. init the logging mechanism
   // 1. 初始化日志机制。
   Logger& logger = Logger::instance();
   logger.setLoggingLevel(QsLogging::TraceLevel);
   const QString sLogPath(QDir(a.applicationDirPath()).filePath("log.txt"));

   // 2. add two destinations
   // 2. 添加两个输出目标。
   DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
     sLogPath, EnableLogRotation, MaxSizeBytes(512), MaxOldLogCount(2)));
   DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
   DestinationPtr functorDestination(DestinationFactory::MakeFunctorDestination(&logFunction));
   logger.addDestination(debugDestination);
   logger.addDestination(fileDestination);
   logger.addDestination(functorDestination);

   // 3. start logging
   // 3. 开始记录日志。
   QLOG_INFO() << "Program started";
   QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();

   QLOG_TRACE() << "Here's a" << QString::fromUtf8("trace") << "message";
   QLOG_DEBUG() << "Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
   QLOG_WARN()  << "Uh-oh!";
   qDebug() << "This message won't be picked up by the logger";
   QLOG_ERROR() << "An error has occurred";
   qWarning() << "Neither will this one";
   QLOG_FATAL() << "Fatal error!";

   logger.setLoggingLevel(QsLogging::OffLevel);
   for (int i = 0;i < 10000000;++i) {
       QLOG_ERROR() << QString::fromUtf8("this message should not be visible");
   }
   logger.setLoggingLevel(QsLogging::TraceLevel);

   // 4. log from a shared library - should automatically share the same log instance as above
   // 4. 从共享库记录日志——应自动共享上面使用的同一个日志实例。
   QLibrary myLib("log_example_shared");
   typedef LogExampleShared* (*LogExampleGetter)();
   typedef void(*LogExampleDeleter)(LogExampleShared*);
   LogExampleGetter fLogCreator = (LogExampleGetter) myLib.resolve("createExample");
   LogExampleDeleter fLogDeleter = (LogExampleDeleter)myLib.resolve("destroyExample");
   LogExampleShared *logFromShared = 0;
   if (fLogCreator && fLogDeleter) {
       logFromShared = fLogCreator();
       logFromShared->logSomething();
       fLogDeleter(logFromShared);
   } else if (!fLogCreator || !fLogDeleter) {
       QLOG_ERROR() << "could not resolve shared library function(s)";
   }

   QLOG_DEBUG() << "Program ending";

   QsLogging::Logger::destroyInstance();
   return 0;
}
