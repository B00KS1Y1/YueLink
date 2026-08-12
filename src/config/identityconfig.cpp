/**
 * @file identityconfig.cpp
 * @brief 实现本机设备身份配置的规范化与校验策略。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-09
 */

#include "identityconfig.h"

#include "configpolicyutils_p.h"

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QUuid>

namespace
{
constexpr qsizetype MaximumDisplayNameLength = 64;
constexpr qsizetype MaximumAvatarPathLength = 4096;
} // namespace

namespace Config
{

void IdentityConfig::normalize(IdentityConfig &config)
{
    QString deviceId = QString::fromStdString(config.device_id).trimmed();
    if (!deviceId.isEmpty())
    {
        const QUuid uuid(deviceId);
        if (!uuid.isNull())
        {
            deviceId = uuid.toString(QUuid::WithoutBraces);
        }
    }
    config.device_id = deviceId.toStdString();
    config.display_name = QString::fromStdString(config.display_name).trimmed().toStdString();

    QString avatarPath = QDir::fromNativeSeparators(QString::fromStdString(config.avatar_path).trimmed());
    if (!avatarPath.isEmpty())
    {
        const QFileInfo avatarInfo(avatarPath);
        avatarPath = QDir::cleanPath(avatarInfo.isAbsolute() ? avatarPath : avatarInfo.absoluteFilePath());
    }
    config.avatar_path = avatarPath.toStdString();
    Detail::normalizeColor(config.avatar_color);
}

QList<Issue> IdentityConfig::validate(const IdentityConfig &config)
{
    QList<Issue> issues;
    const QString deviceId = QString::fromStdString(config.device_id);
    if (!deviceId.isEmpty())
    {
        const QUuid uuid(deviceId);
        if (uuid.isNull() || uuid.toString(QUuid::WithoutBraces) != deviceId)
        {
            issues.append(Detail::makeIssue(QStringLiteral("device_id"), QStringLiteral("设备标识必须为空或使用规范 UUID 格式。")));
        }
    }

    const QString displayName = QString::fromStdString(config.display_name);
    if (displayName.size() > MaximumDisplayNameLength)
    {
        issues.append(Detail::makeIssue(QStringLiteral("display_name"), QStringLiteral("展示名称不能超过 64 个字符。")));
    }

    const QString avatarPath = QString::fromStdString(config.avatar_path);
    if (avatarPath.size() > MaximumAvatarPathLength)
    {
        issues.append(Detail::makeIssue(QStringLiteral("avatar_path"), QStringLiteral("头像路径不能超过 4096 个字符。")));
    }
    else if (!avatarPath.isEmpty() && !QFileInfo(avatarPath).isAbsolute())
    {
        issues.append(Detail::makeIssue(QStringLiteral("avatar_path"), QStringLiteral("头像路径必须为空或使用绝对路径。")));
    }

    if (!Detail::isColor(config.avatar_color))
    {
        issues.append(Detail::makeIssue(QStringLiteral("avatar_color"), QStringLiteral("头像颜色必须使用 #RRGGBB 格式。")));
    }
    return issues;
}

} // namespace Config
