#ifndef QSLOG_H
#define QSLOG_H

#include "QsLogLevel.h"
#include "QsLogDest.h"
#include <QDebug>
#include <QString>

#define QS_LOG_VERSION "2.0b3"

namespace QsLogging
{
class Destination;
class LoggerImpl; // d pointer / d 指针

class QSLOG_SHARED_OBJECT Logger
{
public:
    static Logger& instance();
    static void destroyInstance();
    static Level levelFromLogMessage(const QString& logMessage, bool* conversionSucceeded = 0);

    ~Logger();

    //! Adds a log message destination. Don't add null destinations.
    //! 添加日志消息输出目标；不要添加空目标。
    void addDestination(DestinationPtr destination);
    //! Logging at a level < 'newLevel' will be ignored
    //! 低于“newLevel”级别的日志将被忽略。
    void setLoggingLevel(Level newLevel);
    //! The default level is INFO
    //! 默认级别为 INFO。
    Level loggingLevel() const;
    //! Set to false to disable timestamp inclusion in log messages
    //! 设为 false 可禁止在日志消息中包含时间戳。
    void setIncludeTimestamp(bool e);
    //! Default value is true.
    //! 默认值为 true。
    bool includeTimestamp() const;
    //! Set to false to disable log level inclusion in log messages
    //! 设为 false 可禁止在日志消息中包含日志级别。
    void setIncludeLogLevel(bool l);
    //! Default value is true.
    //! 默认值为 true。
    bool includeLogLevel() const;
    //! Set to true to prepend the source file and line number supplied by the logging macro.
    //! 设为 true 可在消息前添加日志宏提供的源文件名和行号。
    void setIncludeSourceLocation(bool enabled);
    //! Default value is false.
    //! 默认值为 false。
    bool includeSourceLocation() const;
    //! Set to true to dispatch log writes through a dedicated single-thread pool.
    //! 设为 true 可通过专用的单线程线程池分派日志写入任务。
    void setUseSeparateThread(bool enabled);
    //! Default value is false.
    //! 默认值为 false。
    bool useSeparateThread() const;

    //! The helper disables automatic string quoting, forwards streaming to QDebug,
    //! and builds the final log message.
    //! 此辅助类会禁用字符串自动加引号，将流式写入转发给 QDebug，并构建最终日志消息。
    class QSLOG_SHARED_OBJECT Helper
    {
    public:
        explicit Helper(Level logLevel, const char* file = 0, int line = 0) :
            level(logLevel),
            qtDebug(&buffer)
        {
            qtDebug.noquote();
            if (file && Logger::instance().includeSourceLocation())
                qtDebug << file << '@' << line;
        }
        ~Helper();
        QDebug& stream(){ return qtDebug; }

    private:
        void writeToLog();

        Level level;
        QString buffer;
        QDebug qtDebug;
	};

private:
    Logger();
    Logger(const Logger&);            // not available / 不可用
    Logger& operator=(const Logger&); // not available / 不可用

    void enqueueWrite(const QString& message, Level level);
    void write(const QString& message, Level level);

    LoggerImpl* d;

    friend class LogWriterRunnable;
};

} // end namespace / 命名空间结束

#define QLOG_TRACE() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::TraceLevel, __FILE__, __LINE__).stream()
#define QLOG_DEBUG() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::DebugLevel, __FILE__, __LINE__).stream()
#define QLOG_INFO()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::InfoLevel, __FILE__, __LINE__).stream()
#define QLOG_WARN()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::WarnLevel, __FILE__, __LINE__).stream()
#define QLOG_ERROR() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::ErrorLevel, __FILE__, __LINE__).stream()
#define QLOG_FATAL() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::FatalLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::FatalLevel, __FILE__, __LINE__).stream()

#ifdef QS_LOG_DISABLE
#include "QsLogDisableForThisFile.h"
#endif

#endif // QSLOG_H
