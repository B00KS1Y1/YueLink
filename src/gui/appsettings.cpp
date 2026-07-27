#include "appsettings.h"

#include "infrastructure/config/configstore.h"

#include <QColor>
#include <QSet>

#include <spdlog/spdlog.h>

namespace
{
constexpr auto DefaultThemeMode = "dark";
constexpr auto DefaultPrimaryColor = "#4F7CFF";
constexpr auto DefaultLogLevel = "info";
}

AppSettings::AppSettings(QObject *parent)
: QObject(parent)
{
    const Config::ThemeConfig theme = Config::theme.get();
    const Config::ApplicationConfig application = Config::application.get();
    const Config::LogConfig log = Config::log.get();
    m_themeMode = normalizedThemeMode(QString::fromStdString(theme.mode));
    m_primaryColor = QColor(QString::fromStdString(theme.primary_color)).isValid()
                         ? QColor(QString::fromStdString(theme.primary_color))
                               .name(QColor::HexRgb)
                               .toUpper()
                         : QString::fromLatin1(DefaultPrimaryColor);
    m_animationsEnabled = theme.animations_enabled;
    m_notificationsEnabled = application.notifications_enabled;
    m_logLevel = normalizedLogLevel(QString::fromStdString(log.level));
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

bool AppSettings::notificationsEnabled() const
{
    return m_notificationsEnabled;
}

QString AppSettings::logLevel() const
{
    return m_logLevel;
}

QString AppSettings::lastError() const
{
    return m_lastError;
}

bool AppSettings::save(const QString &themeMode,
                       const QString &primaryColor,
                       bool animationsEnabled,
                       bool notificationsEnabled,
                       const QString &logLevel)
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

    const QString normalizedColor = color.name(QColor::HexRgb).toUpper();
    const Config::ThemeConfig previousTheme = Config::theme.get();
    const Config::ApplicationConfig previousApplication = Config::application.get();
    const Config::LogConfig previousLog = Config::log.get();

    Config::ThemeConfig newTheme = previousTheme;
    newTheme.mode = normalizedMode.toStdString();
    newTheme.primary_color = normalizedColor.toStdString();
    newTheme.animations_enabled = animationsEnabled;
    Config::theme.set(newTheme);
    const Config::Result themeResult = Config::theme.save();
    if (!themeResult)
    {
        Config::theme.set(previousTheme);
        setLastError(tr("无法保存外观设置：%1").arg(themeResult.errorMessage));
        spdlog::error("[设置] 保存外观设置失败 原因={}",
                      themeResult.errorMessage.toUtf8().toStdString());
        return false;
    }

    Config::ApplicationConfig newApplication = previousApplication;
    newApplication.notifications_enabled = notificationsEnabled;
    Config::application.set(newApplication);
    const Config::Result applicationResult = Config::application.save();
    if (!applicationResult)
    {
        Config::application.set(previousApplication);
        Config::theme.set(previousTheme);
        const Config::Result rollbackResult = Config::theme.save();
        QString error = tr("无法保存通知设置：%1").arg(applicationResult.errorMessage);
        if (!rollbackResult)
        {
            error += tr("；外观设置回滚失败：%1").arg(rollbackResult.errorMessage);
        }
        setLastError(error);
        spdlog::error("[设置] 保存通知设置失败 原因={}",
                      applicationResult.errorMessage.toUtf8().toStdString());
        return false;
    }

    Config::LogConfig newLog = previousLog;
    newLog.level = normalizedLevel.toStdString();
    Config::log.set(newLog);
    const Config::Result logResult = Config::log.save();
    if (!logResult)
    {
        Config::log.set(previousLog);
        Config::application.set(previousApplication);
        Config::theme.set(previousTheme);
        const Config::Result applicationRollbackResult = Config::application.save();
        const Config::Result themeRollbackResult = Config::theme.save();
        QString error = tr("无法保存日志设置：%1").arg(logResult.errorMessage);
        if (!applicationRollbackResult)
        {
            error += tr("；通知设置回滚失败：%1")
                         .arg(applicationRollbackResult.errorMessage);
        }
        if (!themeRollbackResult)
        {
            error += tr("；外观设置回滚失败：%1")
                         .arg(themeRollbackResult.errorMessage);
        }
        setLastError(error);
        spdlog::error("[设置] 保存日志设置失败 原因={}",
                      logResult.errorMessage.toUtf8().toStdString());
        return false;
    }

    const bool changed = m_themeMode != normalizedMode
                         || m_primaryColor != normalizedColor
                         || m_animationsEnabled != animationsEnabled
                         || m_notificationsEnabled != notificationsEnabled
                         || m_logLevel != normalizedLevel;
    m_themeMode = normalizedMode;
    m_primaryColor = normalizedColor;
    m_animationsEnabled = animationsEnabled;
    m_notificationsEnabled = notificationsEnabled;
    m_logLevel = normalizedLevel;
    spdlog::set_level(spdlog::level::from_str(normalizedLevel.toStdString()));
    setLastError({});
    if (changed)
    {
        emit settingsChanged();
    }
    spdlog::info("[设置] 应用设置已保存 主题={} 动画={} 通知={} 日志级别={}",
                 normalizedMode.toStdString(),
                 animationsEnabled,
                 notificationsEnabled,
                 normalizedLevel.toStdString());
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
    static const QSet<QString> supportedModes = {
        QStringLiteral("light"),
        QStringLiteral("dark"),
        QStringLiteral("system")
    };
    return supportedModes.contains(normalized)
               ? normalized
               : QString::fromLatin1(DefaultThemeMode);
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
    static const QSet<QString> supportedLevels = {
        QStringLiteral("trace"),
        QStringLiteral("debug"),
        QStringLiteral("info"),
        QStringLiteral("warn"),
        QStringLiteral("error"),
        QStringLiteral("critical"),
        QStringLiteral("off")
    };
    return supportedLevels.contains(normalized)
               ? normalized
               : QString::fromLatin1(DefaultLogLevel);
}
