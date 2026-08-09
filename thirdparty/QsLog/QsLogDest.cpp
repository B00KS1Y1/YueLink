#include "QsLogDest.h"
#include "QsLogDestConsole.h"
#include "QsLogDestFile.h"
#include "QsLogDestFunctor.h"
#include <QString>

namespace QsLogging
{

Destination::~Destination()
{
}

//! destination factory
//! 日志输出目标工厂。
DestinationPtr DestinationFactory::MakeFileDestination(const QString& filePath,
    LogRotationOption rotation, const MaxSizeBytes &sizeInBytesToRotateAfter,
    const MaxOldLogCount &oldLogsToKeep)
{
    if (EnableLogRotation == rotation) {
        QScopedPointer<SizeRotationStrategy> logRotation(new SizeRotationStrategy);
        logRotation->setMaximumSizeInBytes(sizeInBytesToRotateAfter.size);
        logRotation->setBackupCount(oldLogsToKeep.count);

        return DestinationPtr(new FileDestination(filePath, RotationStrategyPtr(logRotation.take())));
    }

    return DestinationPtr(new FileDestination(filePath, RotationStrategyPtr(new NullRotationStrategy)));
}

DestinationPtr DestinationFactory::MakeDebugOutputDestination()
{
    return DestinationPtr(new DebugOutputDestination);
}

DestinationPtr DestinationFactory::MakeFunctorDestination(QsLogging::Destination::LogFunction f)
{
    return DestinationPtr(new FunctorDestination(f));
}

DestinationPtr DestinationFactory::MakeFunctorDestination(QObject *receiver, const char *member)
{
    return DestinationPtr(new FunctorDestination(receiver, member));
}

} // end namespace / 命名空间结束
