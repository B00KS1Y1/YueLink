#include "log_example_shared.h"
#include "QsLog.h"

void LogExampleShared::logSomething()
{
    QLOG_INFO() << "this message is comming from a shared library";
}

LogExampleShared* createExample()
{
    return new LogExampleShared();
}


void destroyExample(LogExampleShared *example)
{
    delete example;
}
