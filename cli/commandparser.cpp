#include "commandparser.h"

#include <utility>

namespace Cli
{

ParsedCommand parseCommand(const QString &line)
{
    ParsedCommand result;
    QStringList tokens;
    QString token;
    QChar quote;
    bool tokenStarted = false;

    for (qsizetype index = 0; index < line.size(); ++index)
    {
        const QChar character = line.at(index);
        if (character == QLatin1Char('\\'))
        {
            if (index + 1 < line.size())
            {
                const QChar next = line.at(index + 1);
                if (next == QLatin1Char('\\')
                    || next == QLatin1Char('\'')
                    || next == QLatin1Char('"')
                    || (quote.isNull() && next.isSpace()))
                {
                    token.append(next);
                    tokenStarted = true;
                    ++index;
                    continue;
                }
            }
            token.append(character);
            tokenStarted = true;
            continue;
        }
        if (!quote.isNull())
        {
            if (character == quote)
            {
                quote = {};
            }
            else
            {
                token.append(character);
                tokenStarted = true;
            }
            continue;
        }
        if (character == QLatin1Char('\'') || character == QLatin1Char('"'))
        {
            quote = character;
            tokenStarted = true;
            continue;
        }
        if (character.isSpace())
        {
            if (tokenStarted)
            {
                tokens.append(token);
                token.clear();
                tokenStarted = false;
            }
            continue;
        }
        token.append(character);
        tokenStarted = true;
    }

    if (!quote.isNull())
    {
        result.error = QStringLiteral("命令中存在未闭合的引号。");
        return result;
    }
    if (tokenStarted)
    {
        tokens.append(token);
    }
    if (tokens.isEmpty())
    {
        return result;
    }

    result.name = tokens.takeFirst().toLower();
    result.arguments = std::move(tokens);
    return result;
}

} // namespace Cli
