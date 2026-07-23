#include "consolerenderer.h"

#include <QIODevice>
#include <QJsonDocument>

#include <cstdio>

ConsoleRenderer::ConsoleRenderer(bool jsonLines)
: m_output(stdout, QIODevice::WriteOnly)
, m_jsonLines(jsonLines)
{
}

bool ConsoleRenderer::jsonLines() const
{
    return m_jsonLines;
}

void ConsoleRenderer::line(const QString &text)
{
    if (m_jsonLines)
    {
        event(QStringLiteral("output"), {{QStringLiteral("text"), text}});
        return;
    }
    m_output << text << Qt::endl;
}

void ConsoleRenderer::error(const QString &code, const QString &message)
{
    if (m_jsonLines)
    {
        event(QStringLiteral("error"),
              {{QStringLiteral("code"), code},
               {QStringLiteral("message"), message}});
        return;
    }
    m_output << QStringLiteral("错误 [%1]：%2").arg(code, message) << Qt::endl;
}

void ConsoleRenderer::event(const QString &type, QJsonObject data)
{
    if (m_jsonLines)
    {
        data.insert(QStringLiteral("type"), type);
        m_output << QString::fromUtf8(
                        QJsonDocument(data).toJson(QJsonDocument::Compact))
                 << Qt::endl;
        return;
    }

    const QString message = data.value(QStringLiteral("message")).toString();
    if (!message.isEmpty())
    {
        m_output << message << Qt::endl;
    }
}

void ConsoleRenderer::prompt()
{
    if (m_jsonLines)
    {
        return;
    }
    m_output << QStringLiteral("yuelink> ") << Qt::flush;
}
