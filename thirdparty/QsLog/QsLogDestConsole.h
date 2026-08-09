#ifndef QSLOGDESTCONSOLE_H
#define QSLOGDESTCONSOLE_H

#include "QsLogDest.h"

class QString;

class QsDebugOutput
{
public:
   static void output(const QString& a_message);
};

namespace QsLogging
{

// debugger sink
// 调试器输出目标。
class DebugOutputDestination : public Destination
{
public:
    void write(const QString& message, Level level) override;
    bool isValid() override;
};

}

#endif // QSLOGDESTCONSOLE_H
