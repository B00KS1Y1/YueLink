/**
 * @file commandparser.h
 * @brief 声明交互式 CLI 命令解析功能。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

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

/**
 * @brief 将一行终端输入解析为命令和参数。
 * @param line 用户输入的原始文本。
 * @return 解析后的命令；语法无效时包含错误说明。
 */
[[nodiscard]] ParsedCommand parseCommand(const QString &line);

} // namespace Cli

#endif // COMMANDPARSER_H
