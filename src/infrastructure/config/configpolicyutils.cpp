/**
 * @file configpolicyutils.cpp
 * @brief 实现配置策略共享的私有规范化与校验工具。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-09
 */

#include "configpolicyutils_p.h"

#include <QRegularExpression>

#include <utility>

namespace Config::Detail
{

Issue makeIssue(QString fieldPath, QString message)
{
    return {std::move(fieldPath), std::move(message)};
}

QString normalizedName(const std::string &value)
{
    return QString::fromStdString(value).trimmed().toLower();
}

bool isColor(const std::string &value, bool allowEmpty)
{
    const QString color = QString::fromStdString(value);
    if (allowEmpty && color.isEmpty())
    {
        return true;
    }
    static const QRegularExpression expression(QStringLiteral("^#[0-9A-F]{6}$"));
    return expression.match(color).hasMatch();
}

void normalizeColor(std::string &value)
{
    value = QString::fromStdString(value).trimmed().toUpper().toStdString();
}

} // namespace Config::Detail
