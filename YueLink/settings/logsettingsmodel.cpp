/**
 * @file logsettingsmodel.cpp
 * @brief 实现高级日志设置的自动保存与生效状态管理。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#include "logsettingsmodel.h"

#include "config/configapi.h"
#include "infrastructure/logging.h"

#include <QyLog.h>

LogSettingsModel::LogSettingsModel(QObject *parent)
: SettingsModelBase(parent)
{
    refreshFromConfig();
    m_appliedFilePath = m_filePath;
    m_appliedSourceLocation = m_sourceLocationEnabled;
    m_appliedSeparateThread = m_separateThreadEnabled;
    m_baselineInitialized = true;
    updateRestartRequired();
}

QString LogSettingsModel::level() const
{
    return m_level;
}

QString LogSettingsModel::filePath() const
{
    return m_filePath;
}

bool LogSettingsModel::sourceLocationEnabled() const
{
    return m_sourceLocationEnabled;
}

bool LogSettingsModel::separateThreadEnabled() const
{
    return m_separateThreadEnabled;
}

bool LogSettingsModel::restartRequired() const
{
    return m_restartRequired;
}

bool LogSettingsModel::updateLevel(const QString &level)
{
    const QString normalizedLevel = level.trimmed().toLower();
    if (m_level == normalizedLevel)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::LogConfig>([&normalizedLevel](Config::LogConfig &config) {
        config.level = normalizedLevel.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存日志级别：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存日志级别失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    Logging::setLevel(m_level.toStdString());
    markSaved();
    return true;
}

bool LogSettingsModel::updateFilePath(const QString &filePath)
{
    if (m_filePath == filePath.trimmed())
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::LogConfig>([&filePath](Config::LogConfig &config) {
        config.file_path = filePath.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存日志文件路径：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存日志文件路径失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool LogSettingsModel::updateSourceLocationEnabled(bool enabled)
{
    if (m_sourceLocationEnabled == enabled)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::LogConfig>([enabled](Config::LogConfig &config) {
        config.source_location_enabled = enabled;
    });
    if (!result)
    {
        const QString error = tr("无法保存源码位置设置：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存源码位置设置失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool LogSettingsModel::updateSeparateThreadEnabled(bool enabled)
{
    if (m_separateThreadEnabled == enabled)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::LogConfig>([enabled](Config::LogConfig &config) {
        config.separate_thread_enabled = enabled;
    });
    if (!result)
    {
        const QString error = tr("无法保存独立线程设置：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存独立线程设置失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

void LogSettingsModel::refreshFromConfig()
{
    const Config::LogConfig config = Config::value<Config::LogConfig>();
    const QString level = QString::fromStdString(config.level);
    const QString filePath = QString::fromStdString(config.file_path);
    bool changed = false;

    if (m_level != level)
    {
        m_level = level;
        changed = true;
        emit levelChanged();
    }
    if (m_filePath != filePath)
    {
        m_filePath = filePath;
        changed = true;
        emit filePathChanged();
    }
    if (m_sourceLocationEnabled != config.source_location_enabled)
    {
        m_sourceLocationEnabled = config.source_location_enabled;
        changed = true;
        emit sourceLocationEnabledChanged();
    }
    if (m_separateThreadEnabled != config.separate_thread_enabled)
    {
        m_separateThreadEnabled = config.separate_thread_enabled;
        changed = true;
        emit separateThreadEnabledChanged();
    }

    updateRestartRequired();
    if (changed)
    {
        emit settingsChanged();
    }
}

void LogSettingsModel::updateRestartRequired()
{
    if (!m_baselineInitialized)
    {
        return;
    }

    const bool restartRequired =
        m_filePath != m_appliedFilePath || m_sourceLocationEnabled != m_appliedSourceLocation || m_separateThreadEnabled != m_appliedSeparateThread;
    if (m_restartRequired == restartRequired)
    {
        return;
    }
    m_restartRequired = restartRequired;
    emit restartRequiredChanged();
}
