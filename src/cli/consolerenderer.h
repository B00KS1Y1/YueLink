#ifndef CONSOLERENDERER_H
#define CONSOLERENDERER_H

#include <QJsonObject>
#include <QString>
#include <QTextStream>

class ConsoleRenderer final
{
public:
    explicit ConsoleRenderer(bool jsonLines);

    [[nodiscard]] bool jsonLines() const;
    void line(const QString &text = {});
    void error(const QString &code, const QString &message);
    void event(const QString &type, QJsonObject data = {});
    void prompt();

private:
    QTextStream m_output;
    bool m_jsonLines = false;
};

#endif // CONSOLERENDERER_H
