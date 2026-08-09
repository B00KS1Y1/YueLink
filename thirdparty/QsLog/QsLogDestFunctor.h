#ifndef QSLOGDESTFUNCTOR_H
#define QSLOGDESTFUNCTOR_H

#include "QsLogDest.h"
#include <QObject>

namespace QsLogging
{
// Offers various types of function-like sinks.
// This is an advanced destination type. Depending on your configuration, LogFunction might be
// called from a different thread or even a different binary. You should not access QsLog from
// inside LogFunction and should not perform any time-consuming operations.
// logMessageReady is connected through a queued connection and trace messages are not included
// 提供多种类似函数的输出目标。
// 这是一种高级输出目标类型。根据配置，LogFunction 可能会从其他线程甚至其他二进制模块中调用。
// 不应在 LogFunction 内部访问 QsLog，也不应执行任何耗时操作。
// logMessageReady 使用队列连接，并且不包含跟踪级别的消息。
class FunctorDestination : public QObject, public Destination
{
    Q_OBJECT
public:
    explicit FunctorDestination(LogFunction f);
    FunctorDestination(QObject *receiver, const char *member);

    void write(const QString &message, Level level) override;
    bool isValid() override;

protected:
    // int used to avoid registering a new enum type
    // 使用 int 以避免注册新的枚举类型。
    Q_SIGNAL void logMessageReady(const QString &message, int level);

private:
    LogFunction mLogFunction;
};
}

#endif // QSLOGDESTFUNCTOR_H
