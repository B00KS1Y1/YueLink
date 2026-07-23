#include "consoleinput.h"

#include <QIODevice>
#include <QPointer>
#include <QTextStream>
#include <QThread>

#include <cstdio>

ConsoleInput::ConsoleInput(QObject *parent)
: QObject(parent)
{
}

void ConsoleInput::start()
{
    if (m_started)
    {
        return;
    }
    m_started = true;

    const QPointer<ConsoleInput> guard(this);
    QThread *thread = QThread::create([guard]() {
        QTextStream input(stdin, QIODevice::ReadOnly);
        while (guard)
        {
            const QString line = input.readLine();
            if (line.isNull())
            {
                if (guard)
                {
                    QMetaObject::invokeMethod(
                        guard,
                        [guard]() {
                            if (guard)
                            {
                                emit guard->inputClosed();
                            }
                        },
                        Qt::QueuedConnection);
                }
                return;
            }
            if (guard)
            {
                QMetaObject::invokeMethod(
                    guard,
                    [guard, line]() {
                        if (guard)
                        {
                            emit guard->lineRead(line);
                        }
                    },
                    Qt::QueuedConnection);
            }
        }
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}
