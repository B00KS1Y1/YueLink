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
#include <QUrl>

#include <cmath>

namespace Config
{

ThemeConfig::ThemeConfig()
: background_images{
      {"雾蓝樱粉（内置）", "qrc:/yuelink/assets/backgrounds/mist-blue-sakura-pink.png", true},
      {"湖蓝柔粉（内置）", "qrc:/yuelink/assets/backgrounds/lake-blue-soft-pink.png", true},
      {"鼠尾草浅黄（内置）", "qrc:/yuelink/assets/backgrounds/sage-light-yellow.png", true},
      {"薄荷孔雀绿（内置）", "qrc:/yuelink/assets/backgrounds/mint-peacock-green.png", true},
      {"玫红雪白（内置）", "qrc:/yuelink/assets/backgrounds/rose-red-snow-white.png", true},
      {"浅蓝薄荷（内置）", "qrc:/yuelink/assets/backgrounds/pale-blue-mint.png", true},
      {"夜紫橙金（内置）", "qrc:/yuelink/assets/backgrounds/night-violet-orange-gold.png", true},
      {"深蓝柔粉（内置）", "qrc:/yuelink/assets/backgrounds/deep-blue-soft-pink.png", true},
      {"青绿奶杏（内置）", "qrc:/yuelink/assets/backgrounds/aqua-green-almond.png", true},
      {"薄荷橙金（内置）", "qrc:/yuelink/assets/backgrounds/mint-orange-gold.png", true},
      {"荧光水绿（内置）", "qrc:/yuelink/assets/backgrounds/neon-aqua-green.png", true},
      {"珊瑚玫瑰雾（内置）", "qrc:/yuelink/assets/backgrounds/coral-rose-mist.png", true},
      {"天蓝奶黄（内置）", "qrc:/yuelink/assets/backgrounds/sky-blue-cream-yellow.png", true},
      {"雾青浅绿（内置）", "qrc:/yuelink/assets/backgrounds/misty-cyan-green.png", true},
      {"深绿薄雾（内置）", "qrc:/yuelink/assets/backgrounds/deep-green-mist.png", true},
      {"电光蓝紫（内置）", "qrc:/yuelink/assets/backgrounds/electric-blue-violet.png", true},
  }
{
}

void ThemeConfig::normalize(ThemeConfig &config)
{
    config.mode = Detail::normalizedName(config.mode).toStdString();
    Detail::normalizeColor(config.primary_color);
    config.background_image = QString::fromStdString(config.background_image).trimmed().toStdString();
    for (BackgroundImageConfig &background : config.background_images)
    {
        background.name = QString::fromStdString(background.name).trimmed().toStdString();
        background.source = QString::fromStdString(background.source).trimmed().toStdString();
    }
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
    const QString backgroundImage = QString::fromStdString(config.background_image);
    const QUrl backgroundUrl(backgroundImage);
    if (!backgroundImage.isEmpty() &&
        (!backgroundUrl.isValid() || (!backgroundUrl.isLocalFile() && backgroundUrl.scheme().compare(QStringLiteral("qrc"), Qt::CaseInsensitive) != 0)))
    {
        issues.append(Detail::makeIssue(QStringLiteral("background_image"), QStringLiteral("背景图片必须使用 qrc URL 或本地 file URL。")));
    }
    if (!std::isfinite(config.background_opacity) || config.background_opacity < 0.0 || config.background_opacity > 1.0)
    {
        issues.append(Detail::makeIssue(QStringLiteral("background_opacity"), QStringLiteral("背景透明度必须在 0.0 到 1.0 之间。")));
    }
    if (config.background_images.size() > 64)
    {
        issues.append(Detail::makeIssue(QStringLiteral("background_images"), QStringLiteral("最多只能保存 64 张背景图片。")));
    }
    if (config.background_images.empty())
    {
        issues.append(Detail::makeIssue(QStringLiteral("background_images"), QStringLiteral("至少需要保留一张背景图片。")));
    }
    QSet<QString> backgroundNames;
    QSet<QString> backgroundSources;
    bool hasEnabledBackground = false;
    bool currentBackgroundExists = false;
    for (qsizetype index = 0; index < static_cast<qsizetype>(config.background_images.size()); ++index)
    {
        const BackgroundImageConfig &background = config.background_images.at(static_cast<std::size_t>(index));
        const QString name = QString::fromStdString(background.name);
        const QUrl source(QString::fromStdString(background.source));
        const QString fieldPrefix = QStringLiteral("background_images[%1]").arg(index);
        if (name.isEmpty() || name.size() > 64)
        {
            issues.append(Detail::makeIssue(fieldPrefix + QStringLiteral(".name"), QStringLiteral("背景名称必须包含 1 到 64 个字符。")));
        }
        const QString normalizedName = name.toCaseFolded();
        if (!normalizedName.isEmpty() && backgroundNames.contains(normalizedName))
        {
            issues.append(Detail::makeIssue(fieldPrefix + QStringLiteral(".name"), QStringLiteral("背景名称不能重复。")));
        }
        backgroundNames.insert(normalizedName);
        const bool isResource = source.scheme().compare(QStringLiteral("qrc"), Qt::CaseInsensitive) == 0;
        if (!source.isValid() || (!source.isLocalFile() && !isResource))
        {
            issues.append(Detail::makeIssue(fieldPrefix + QStringLiteral(".source"), QStringLiteral("背景图片必须使用 qrc URL 或本地 file URL。")));
        }
        const QString normalizedSource = source.adjusted(QUrl::NormalizePathSegments).toString(QUrl::FullyEncoded);
        if (!normalizedSource.isEmpty() && backgroundSources.contains(normalizedSource))
        {
            issues.append(Detail::makeIssue(fieldPrefix + QStringLiteral(".source"), QStringLiteral("背景图片地址不能重复。")));
        }
        backgroundSources.insert(normalizedSource);
        hasEnabledBackground = hasEnabledBackground || background.enabled;
        currentBackgroundExists = currentBackgroundExists ||
                                  (background.enabled && source.adjusted(QUrl::NormalizePathSegments) == backgroundUrl.adjusted(QUrl::NormalizePathSegments));
    }
    if (!config.background_images.empty() && !hasEnabledBackground)
    {
        issues.append(Detail::makeIssue(QStringLiteral("background_images"), QStringLiteral("至少需要启用一张背景图片。")));
    }
    if (!config.background_images.empty() && !currentBackgroundExists)
    {
        issues.append(Detail::makeIssue(QStringLiteral("background_image"), QStringLiteral("当前背景必须是已启用的背景图片。")));
    }
    return issues;
}

} // namespace Config
