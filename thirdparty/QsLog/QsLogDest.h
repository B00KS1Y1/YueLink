#ifndef QSLOGDEST_H
#define QSLOGDEST_H

#include "QsLogLevel.h"
#include <QSharedPointer>
#include <QtGlobal>
class QString;
class QObject;

#ifdef QSLOG_IS_SHARED_LIBRARY
#define QSLOG_SHARED_OBJECT Q_DECL_EXPORT
#elif QSLOG_IS_SHARED_LIBRARY_IMPORT
#define QSLOG_SHARED_OBJECT Q_DECL_IMPORT
#else
#define QSLOG_SHARED_OBJECT
#endif

namespace QsLogging
{

class QSLOG_SHARED_OBJECT Destination
{
public:
    typedef void (*LogFunction)(const QString &message, Level level);

public:
    virtual ~Destination();
    virtual void write(const QString& message, Level level) = 0;
    virtual bool isValid() = 0; // returns whether the destination was created correctly / 返回输出目标是否已正确创建
};
typedef QSharedPointer<Destination> DestinationPtr;


// a series of "named" paramaters, to make the file destination creation more readable
// 一组“具名”参数，用于提高文件输出目标创建代码的可读性。
enum LogRotationOption
{
    DisableLogRotation = 0,
    EnableLogRotation  = 1
};

struct QSLOG_SHARED_OBJECT MaxSizeBytes
{
    MaxSizeBytes() : size(0) {}
    explicit MaxSizeBytes(qint64 size_) : size(size_) {}
    qint64 size;
};

struct QSLOG_SHARED_OBJECT MaxOldLogCount
{
    MaxOldLogCount() : count(0) {}
    explicit MaxOldLogCount(int count_) : count(count_) {}
    int count;
};


//! Creates logging destinations/sinks. The caller shares ownership of the destinations with the logger.
//! After being added to a logger, the caller can discard the pointers.
//! 创建日志输出目标/接收端。调用方与日志记录器共享输出目标的所有权；
//! 将目标添加到日志记录器后，调用方可以丢弃相应指针。
class QSLOG_SHARED_OBJECT DestinationFactory
{
public:
    static DestinationPtr MakeFileDestination(const QString& filePath,
        LogRotationOption rotation = DisableLogRotation,
        const MaxSizeBytes &sizeInBytesToRotateAfter = MaxSizeBytes(),
        const MaxOldLogCount &oldLogsToKeep = MaxOldLogCount());
    static DestinationPtr MakeDebugOutputDestination();
    // takes a pointer to a function
    // 接收一个函数指针。
    static DestinationPtr MakeFunctorDestination(Destination::LogFunction f);
    // takes a QObject + signal/slot
    // 接收一个 QObject 以及信号/槽成员。
    static DestinationPtr MakeFunctorDestination(QObject *receiver, const char *member);
};

} // end namespace / 命名空间结束

#endif // QSLOGDEST_H
