/**
 * @file applicationsettingsmodel.cpp
 * @brief 实现通用设置的自动保存与变更通知。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#include "applicationsettingsmodel.h"

#include "config/configapi.h"

#include <QyLog.h>

ApplicationSettingsModel::ApplicationSettingsModel(QObject *parent)
: SettingsModelBase(parent)
{
    refreshFromConfig();
}

bool ApplicationSettingsModel::notificationsEnabled() const
{
    return m_notificationsEnabled;
}

QString ApplicationSettingsModel::downloadDirectory() const
{
    return m_downloadDirectory;
}

bool ApplicationSettingsModel::updateNotificationsEnabled(bool enabled)
{
    if (m_notificationsEnabled == enabled)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ApplicationConfig>([enabled](Config::ApplicationConfig &config) {
        config.notifications_enabled = enabled;
    });
    if (!result)
    {
        const QString error = tr("无法保存通知设置：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存通知设置失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ApplicationSettingsModel::updateDownloadDirectory(const QString &directory)
{
    if (m_downloadDirectory == directory.trimmed())
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ApplicationConfig>([&directory](Config::ApplicationConfig &config) {
        config.download_directory = directory.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存下载目录：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存下载目录失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

void ApplicationSettingsModel::refreshFromConfig()
{
    const Config::ApplicationConfig config = Config::value<Config::ApplicationConfig>();
    const QString downloadDirectory = QString::fromStdString(config.download_directory);
    bool changed = false;

    if (m_notificationsEnabled != config.notifications_enabled)
    {
        m_notificationsEnabled = config.notifications_enabled;
        changed = true;
        emit notificationsEnabledChanged();
    }
    if (m_downloadDirectory != downloadDirectory)
    {
        m_downloadDirectory = downloadDirectory;
        changed = true;
        emit downloadDirectoryChanged();
    }
    if (changed)
    {
        emit settingsChanged();
    }
}
