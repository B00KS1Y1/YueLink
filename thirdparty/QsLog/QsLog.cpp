#include "QsLog.h"
#include "QsLogDest.h"
#include <QThreadPool>
#include <QRunnable>
#include <QMutex>
#include <QVector>
#include <QDateTime>
#include <QtGlobal>
#include <cstdlib>
#include <stdexcept>

namespace QsLogging
{
typedef QVector<DestinationPtr> DestinationList;

static const char TraceString[] = "TRACE";
static const char DebugString[] = "DEBUG";
static const char InfoString[]  = "INFO ";
static const char WarnString[]  = "WARN ";
static const char ErrorString[] = "ERROR";
static const char FatalString[] = "FATAL";

// not using Qt::ISODate because we need the milliseconds too
// 未使用 Qt::ISODate，因为这里还需要包含毫秒。
static const QString fmtDateTime("yyyy-MM-ddThh:mm:ss.zzz");

static Logger* sInstance = 0;

static const char* LevelToText(Level theLevel)
{
    switch (theLevel) {
        case TraceLevel:
            return TraceString;
        case DebugLevel:
            return DebugString;
        case InfoLevel:
            return InfoString;
        case WarnLevel:
            return WarnString;
        case ErrorLevel:
            return ErrorString;
        case FatalLevel:
            return FatalString;
        case OffLevel:
            return "";
        default: {
            Q_ASSERT(!"bad log level");
            return InfoString;
        }
    }
}

class LogWriterRunnable : public QRunnable
{
public:
    LogWriterRunnable(QString message, Level level);
    virtual void run();

private:
    QString mMessage;
    Level mLevel;
};

class LoggerImpl
{
public:
    LoggerImpl();

    QThreadPool threadPool;
    QMutex logMutex;
    Level level;
    DestinationList destList;
    bool includeTimeStamp;
    bool includeLogLevel;
    bool includeSourceLocation;
    bool useSeparateThread;
};

LogWriterRunnable::LogWriterRunnable(QString message, Level level)
    : QRunnable()
    , mMessage(message)
    , mLevel(level)
{
}

void LogWriterRunnable::run()
{
    Logger::instance().write(mMessage, mLevel);
}


LoggerImpl::LoggerImpl()
    : level(InfoLevel)
    , includeTimeStamp(true)
    , includeLogLevel(true)
    , includeSourceLocation(false)
    , useSeparateThread(false)
{
    // assume at least file + console
    // 假定至少会使用文件和控制台两个输出目标。
    destList.reserve(2);
    threadPool.setMaxThreadCount(1);
    threadPool.setExpiryTimeout(-1);
}


Logger::Logger()
    : d(new LoggerImpl)
{
}

Logger& Logger::instance()
{
    if (!sInstance)
        sInstance = new Logger;

    return *sInstance;
}

void Logger::destroyInstance()
{
    delete sInstance;
    sInstance = 0;
}

// tries to extract the level from a string log message. If available, conversionSucceeded will
// contain the conversion result.
// 尝试从字符串日志消息中提取级别；如果提供了 conversionSucceeded，它将保存转换结果。
Level Logger::levelFromLogMessage(const QString& logMessage, bool* conversionSucceeded)
{
    if (conversionSucceeded)
        *conversionSucceeded = true;

    if (logMessage.startsWith(QLatin1String(TraceString)))
        return TraceLevel;
    if (logMessage.startsWith(QLatin1String(DebugString)))
        return DebugLevel;
    if (logMessage.startsWith(QLatin1String(InfoString)))
        return InfoLevel;
    if (logMessage.startsWith(QLatin1String(WarnString)))
        return WarnLevel;
    if (logMessage.startsWith(QLatin1String(ErrorString)))
        return ErrorLevel;
    if (logMessage.startsWith(QLatin1String(FatalString)))
        return FatalLevel;

    if (conversionSucceeded)
        *conversionSucceeded = false;
    return OffLevel;
}

Logger::~Logger()
{
    d->threadPool.waitForDone();
    delete d;
    d = 0;
}

void Logger::addDestination(DestinationPtr destination)
{
    Q_ASSERT(destination.data());
    d->destList.push_back(destination);
}

void Logger::setLoggingLevel(Level newLevel)
{
    d->level = newLevel;
}

Level Logger::loggingLevel() const
{
    return d->level;
}

void Logger::setIncludeTimestamp(bool e)
{
    d->includeTimeStamp = e;
}

bool Logger::includeTimestamp() const
{
    return d->includeTimeStamp;
}

void Logger::setIncludeLogLevel(bool l)
{
    d->includeLogLevel = l;
}

bool Logger::includeLogLevel() const
{
    return d->includeLogLevel;
}

void Logger::setIncludeSourceLocation(bool enabled)
{
    d->includeSourceLocation = enabled;
}

bool Logger::includeSourceLocation() const
{
    return d->includeSourceLocation;
}

void Logger::setUseSeparateThread(bool enabled)
{
    const bool wasEnabled = d->useSeparateThread;
    d->useSeparateThread = enabled;
    if (wasEnabled && !enabled)
        d->threadPool.waitForDone();
}

bool Logger::useSeparateThread() const
{
    return d->useSeparateThread;
}

//! creates the complete log message and passes it to the logger
//! 创建完整的日志消息并将其传递给日志记录器。
void Logger::Helper::writeToLog()
{
    const char* const levelName = LevelToText(level);
    QString completeMessage;
    Logger &logger = Logger::instance();
    if (logger.includeLogLevel()) {
        completeMessage.
                append(levelName).
                append(' ');
    }
    if (logger.includeTimestamp()) {
        completeMessage.
                append(QDateTime::currentDateTime().toString(fmtDateTime)).
                append(' ');
    }
    completeMessage.append(buffer);
    Logger::instance().enqueueWrite(completeMessage, level);
}

Logger::Helper::~Helper()
{
    try {
        writeToLog();
    }
    catch(std::exception&) {
        // you shouldn't throw exceptions from a sink
        // 不应从日志输出目标中抛出异常。
        Q_ASSERT(!"exception in logger helper destructor");
        throw;
    }
}

//! directs the message to the task queue or writes it directly
//! 将消息送入任务队列，或直接写出。
void Logger::enqueueWrite(const QString& message, Level level)
{
    if (d->useSeparateThread) {
        LogWriterRunnable *r = new LogWriterRunnable(message, level);
        d->threadPool.start(r);
    } else {
        write(message, level);
    }
}

//! Sends the message to all the destinations. The level for this message is passed in case
//! it's useful for processing in the destination.
//! 将消息发送到所有输出目标；同时传入该消息的级别，以便输出目标按需处理。
void Logger::write(const QString& message, Level level)
{
    QMutexLocker lock(&d->logMutex);
    for (DestinationList::iterator it = d->destList.begin(),
        endIt = d->destList.end();it != endIt;++it) {
        (*it)->write(message, level);
    }
}

} // end namespace / 命名空间结束
