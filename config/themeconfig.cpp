/**
 * @file themeconfig.cpp
 * @brief 实现主题配置的规范化与校验策略。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-09
 */

#include "themeconfig.h"

#include "configpolicyutils_p.h"

#include <QSet>
#include <QString>

namespace Config
{

void ThemeConfig::normalize(ThemeConfig &config)
{
    config.mode = Detail::normalizedName(config.mode).toStdString();
    Detail::normalizeColor(config.primary_color);
    Detail::normalizeColor(config.dark_background);
    Detail::normalizeColor(config.light_background);
}

QList<Issue> ThemeConfig::validate(const ThemeConfig &config)
{
    QList<Issue> issues;
    static const QSet<QString> themeModes = {QStringLiteral("light"), QStringLiteral("dark"), QStringLiteral("system")};
    if (!themeModes.contains(QString::fromStdString(config.mode)))
    {
        issues.append(Detail::makeIssue(QStringLiteral("mode"), QStringLiteral("仅支持 light、dark 或 system。")));
    }
    if (!Detail::isColor(config.primary_color))
    {
        issues.append(Detail::makeIssue(QStringLiteral("primary_color"), QStringLiteral("颜色必须使用 #RRGGBB 格式。")));
    }
    if (!Detail::isColor(config.dark_background, true))
    {
        issues.append(Detail::makeIssue(QStringLiteral("dark_background"), QStringLiteral("颜色必须为空或使用 #RRGGBB 格式。")));
    }
    if (!Detail::isColor(config.light_background, true))
    {
        issues.append(Detail::makeIssue(QStringLiteral("light_background"), QStringLiteral("颜色必须为空或使用 #RRGGBB 格式。")));
    }
    return issues;
}

} // namespace Config
