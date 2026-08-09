#include "appsettings.h"

#include "infrastructure/config/configapi.h"

#include <QColor>
#include <QSet>

#include <spdlog/spdlog.h>

#include <utility>

namespace
{
constexpr auto DefaultThemeMode = "dark";
constexpr auto DefaultPrimaryColor = "#4F7CFF";
constexpr auto DefaultNavigationMode = "compact";
constexpr auto DefaultLogLevel = "info";
} // namespace

AppSettings::AppSettings(QObject *parent)
: QObject(parent)
{
    const Config::ThemeConfig theme = Config::value<Config::ThemeConfig>();
    const Config::ApplicationConfig application = Config::value<Config::ApplicationConfig>();
    const Config::LogConfig log = Config::value<Config::LogConfig>();
    m_themeMode = normalizedThemeMode(QString::fromStdString(theme.mode));
    m_primaryColor = QColor(QString::fromStdString(theme.primary_color)).isValid()
                         ? QColor(QString::fromStdString(theme.primary_color)).name(QColor::HexRgb).toUpper()
                         : QString::fromLatin1(DefaultPrimaryColor);
    m_animationsEnabled = theme.animations_enabled;
    m_navigationMode = normalizedNavigationMode(QString::fromStdString(theme.navigation_mode));
    m_notificationsEnabled = application.notifications_enabled;
    m_downloadDirectory = QString::fromStdString(application.download_directory);
    m_logLevel = normalizedLogLevel(QString::fromStdString(log.level));
    m_logFilePath = QString::fromStdString(log.file_path);
}

QString AppSettings::themeMode() const
{
    return m_themeMode;
}

QString AppSettings::primaryColor() const
{
    return m_primaryColor;
}

bool AppSettings::animationsEnabled() const
{
    return m_animationsEnabled;
}

QString AppSettings::navigationMode() const
{
    return m_navigationMode;
}

bool AppSettings::notificationsEnabled() const
{
    return m_notificationsEnabled;
}

QString AppSettings::downloadDirectory() const
{
    return m_downloadDirectory;
}

QString AppSettings::logLevel() const
{
    return m_logLevel;
}

QString AppSettings::logFilePath() const
{
    return m_logFilePath;
}

QString AppSettings::lastError() const
{
    return m_lastError;
}

bool AppSettings::save(const QString &themeMode,
                       const QString &primaryColor,
                       bool animationsEnabled,
                       const QString &navigationMode,
                       bool notificationsEnabled,
                       const QString &downloadDirectory,
                       const QString &logLevel,
                       const QString &logFilePath)
{
    const QString normalizedMode = normalizedThemeMode(themeMode);
    if (normalizedMode != themeMode.trimmed().toLower())
    {
        setLastError(tr("主题模式无效。"));
        return false;
    }

    const QColor color(primaryColor.trimmed());
    if (!color.isValid())
    {
        setLastError(tr("主题色格式无效，请使用十六进制颜色。"));
        return false;
    }

    const QString normalizedLevel = normalizedLogLevel(logLevel);
    if (normalizedLevel != logLevel.trimmed().toLower())
    {
        setLastError(tr("日志级别无效。"));
        return false;
    }

    const QString normalizedNavigation = normalizedNavigationMode(navigationMode);
    if (normalizedNavigation != navigationMode.trimmed().toLower())
    {
        setLastError(tr("导航布局无效。"));
        return false;
    }

    const QString normalizedColor = color.name(QColor::HexRgb).toUpper();
    const Config::ThemeConfig previousTheme = Config::value<Config::ThemeConfig>();
    const Config::ApplicationConfig previousApplication = Config::value<Config::ApplicationConfig>();
    const Config::LogConfig previousLog = Config::value<Config::LogConfig>();

    Config::ThemeConfig newTheme = previousTheme;
    newTheme.mode = normalizedMode.toStdString();
    newTheme.primary_color = normalizedColor.toStdString();
    newTheme.animations_enabled = animationsEnabled;
    newTheme.navigation_mode = normalizedNavigation.toStdString();
    const Config::Result themeResult = Config::set(std::move(newTheme));
    if (!themeResult)
    {
        setLastError(tr("无法保存外观设置：%1").arg(themeResult.errorMessage));
        spdlog::error("[设置] 保存外观设置失败 原因={}", themeResult.errorMessage.toUtf8().toStdString());
        return false;
    }

    Config::ApplicationConfig newApplication = previousApplication;
    newApplication.notifications_enabled = notificationsEnabled;
    newApplication.download_directory = downloadDirectory.toStdString();
    const Config::Result applicationResult = Config::set(std::move(newApplication));
    if (!applicationResult)
    {
        const Config::Result rollbackResult = Config::set(previousTheme);
        QString error = tr("无法保存通知设置：%1").arg(applicationResult.errorMessage);
        if (!rollbackResult)
        {
            error += tr("；外观设置回滚失败：%1").arg(rollbackResult.errorMessage);
        }
        setLastError(error);
        spdlog::error("[设置] 保存通知设置失败 原因={}", applicationResult.errorMessage.toUtf8().toStdString());
        return false;
    }

    Config::LogConfig newLog = previousLog;
    newLog.level = normalizedLevel.toStdString();
    newLog.file_path = logFilePath.toStdString();
    const Config::Result logResult = Config::set(std::move(newLog));
    if (!logResult)
    {
        const Config::Result applicationRollbackResult = Config::set(previousApplication);
        const Config::Result themeRollbackResult = Config::set(previousTheme);
        QString error = tr("无法保存日志设置：%1").arg(logResult.errorMessage);
        if (!applicationRollbackResult)
        {
            error += tr("；通知设置回滚失败：%1").arg(applicationRollbackResult.errorMessage);
        }
        if (!themeRollbackResult)
        {
            error += tr("；外观设置回滚失败：%1").arg(themeRollbackResult.errorMessage);
        }
        setLastError(error);
        spdlog::error("[设置] 保存日志设置失败 原因={}", logResult.errorMessage.toUtf8().toStdString());
        return false;
    }

    const Config::ApplicationConfig savedApplication = Config::value<Config::ApplicationConfig>();
    const Config::LogConfig savedLog = Config::value<Config::LogConfig>();
    const QString savedDownloadDirectory = QString::fromStdString(savedApplication.download_directory);
    const QString savedLogFilePath = QString::fromStdString(savedLog.file_path);
    const bool changed = m_themeMode != normalizedMode || m_primaryColor != normalizedColor || m_animationsEnabled != animationsEnabled ||
                         m_navigationMode != normalizedNavigation || m_notificationsEnabled != notificationsEnabled ||
                         m_downloadDirectory != savedDownloadDirectory || m_logLevel != normalizedLevel || m_logFilePath != savedLogFilePath;
    m_themeMode = normalizedMode;
    m_primaryColor = normalizedColor;
    m_animationsEnabled = animationsEnabled;
    m_navigationMode = normalizedNavigation;
    m_notificationsEnabled = notificationsEnabled;
    m_downloadDirectory = savedDownloadDirectory;
    m_logLevel = normalizedLevel;
    m_logFilePath = savedLogFilePath;
    spdlog::set_level(spdlog::level::from_str(normalizedLevel.toStdString()));
    setLastError({});
    if (changed)
    {
        emit settingsChanged();
    }
    spdlog::info("[设置] 应用设置已保存 主题={} 导航布局={} 动画={} 通知={} 下载目录={} 日志级别={} 日志路径={}",
                 normalizedMode.toStdString(),
                 normalizedNavigation.toStdString(),
                 animationsEnabled,
                 notificationsEnabled,
                 savedDownloadDirectory.toUtf8().toStdString(),
                 normalizedLevel.toStdString(),
                 savedLogFilePath.toUtf8().toStdString());
    return true;
}

void AppSettings::setLastError(const QString &error)
{
    if (m_lastError == error)
    {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

QString AppSettings::normalizedThemeMode(const QString &mode)
{
    const QString normalized = mode.trimmed().toLower();
    static const QSet<QString> supportedModes = {QStringLiteral("light"), QStringLiteral("dark"), QStringLiteral("system")};
    return supportedModes.contains(normalized) ? normalized : QString::fromLatin1(DefaultThemeMode);
}

QString AppSettings::normalizedNavigationMode(const QString &mode)
{
    const QString normalized = mode.trimmed().toLower();
    static const QSet<QString> supportedModes = {QStringLiteral("relaxed"), QStringLiteral("standard"), QStringLiteral("compact")};
    return supportedModes.contains(normalized) ? normalized : QString::fromLatin1(DefaultNavigationMode);
}

QString AppSettings::normalizedLogLevel(const QString &level)
{
    const QString normalized = level.trimmed().toLower();
    if (normalized == QLatin1String("warning"))
    {
        return QStringLiteral("warn");
    }
    if (normalized == QLatin1String("err"))
    {
        return QStringLiteral("error");
    }
    static const QSet<QString> supportedLevels = {QStringLiteral("trace"),
                                                  QStringLiteral("debug"),
                                                  QStringLiteral("info"),
                                                  QStringLiteral("warn"),
                                                  QStringLiteral("error"),
                                                  QStringLiteral("critical"),
                                                  QStringLiteral("off")};
    return supportedLevels.contains(normalized) ? normalized : QString::fromLatin1(DefaultLogLevel);
}
