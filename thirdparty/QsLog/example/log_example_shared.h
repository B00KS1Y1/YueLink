#ifndef LOGEXAMPLESHARED_H
#define LOGEXAMPLESHARED_H

#include <QtGlobal>

#ifdef EXAMPLE_IS_SHARED_LIBRARY
#define EXAMPLE_SHARED_OBJECT Q_DECL_IMPORT
#else
#define EXAMPLE_SHARED_OBJECT Q_DECL_EXPORT
#endif

class EXAMPLE_SHARED_OBJECT LogExampleShared
{
public:
    void logSomething();
};

extern "C" {
    LogExampleShared *createExample();
    void destroyExample(LogExampleShared *example);
}

#endif // LOGEXAMPLESHARED_H
