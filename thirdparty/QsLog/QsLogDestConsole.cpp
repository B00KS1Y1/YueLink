#include "QsLogDestConsole.h"
#include <QString>
#include <QtGlobal>

#if defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
void QsDebugOutput::output( const QString& message )
{
   OutputDebugStringW(reinterpret_cast<const WCHAR*>(message.utf16()));
   OutputDebugStringW(L"\n");
}
#elif defined(Q_OS_UNIX)
#include <cstdio>
void QsDebugOutput::output( const QString& message )
{
   fprintf(stderr, "%s\n", qPrintable(message));
   fflush(stderr);
}
#endif

void QsLogging::DebugOutputDestination::write(const QString& message, Level)
{
    QsDebugOutput::output(message);
}

bool QsLogging::DebugOutputDestination::isValid()
{
    return true;
}
