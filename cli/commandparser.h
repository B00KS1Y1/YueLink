#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QString>
#include <QStringList>

namespace Cli
{

struct ParsedCommand
{
    QString name;
    QStringList arguments;
    QString error;
};

[[nodiscard]] ParsedCommand parseCommand(const QString &line);

} // namespace Cli

#endif // COMMANDPARSER_H
