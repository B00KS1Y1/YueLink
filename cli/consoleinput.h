#ifndef CONSOLEINPUT_H
#define CONSOLEINPUT_H

#include <QObject>

class ConsoleInput final : public QObject
{
    Q_OBJECT

public:
    explicit ConsoleInput(QObject *parent = nullptr);
    void start();

signals:
    void lineRead(const QString &line);
    void inputClosed();

private:
    bool m_started = false;
};

#endif // CONSOLEINPUT_H
