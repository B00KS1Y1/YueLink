/**
 * @file themesettingsmodel.cpp
 * @brief 实现外观设置的自动保存与变更通知。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#include "themesettingsmodel.h"

#include "config/configapi.h"

#include <QColor>

#include <QsLog.h>

ThemeSettingsModel::ThemeSettingsModel(QObject *parent)
: SettingsModelBase(parent)
{
    refreshFromConfig();
}

QString ThemeSettingsModel::mode() const
{
    return m_mode;
}

QString ThemeSettingsModel::primaryColor() const
{
    return m_primaryColor;
}

bool ThemeSettingsModel::animationsEnabled() const
{
    return m_animationsEnabled;
}

bool ThemeSettingsModel::updateMode(const QString &mode)
{
    const QString normalizedMode = mode.trimmed().toLower();
    if (m_mode == normalizedMode)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ThemeConfig>([&normalizedMode](Config::ThemeConfig &config) {
        config.mode = normalizedMode.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存主题模式：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存主题模式失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::updatePrimaryColor(const QString &color)
{
    const QColor parsedColor(color.trimmed());
    if (!parsedColor.isValid())
    {
        const QString error = tr("主题色格式无效，请使用十六进制颜色。");
        markError(error);
        return false;
    }

    const QString normalizedColor = parsedColor.name(QColor::HexRgb).toUpper();
    if (m_primaryColor == normalizedColor)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ThemeConfig>([&normalizedColor](Config::ThemeConfig &config) {
        config.primary_color = normalizedColor.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存主题色：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存主题色失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::updateAnimationsEnabled(bool enabled)
{
    if (m_animationsEnabled == enabled)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ThemeConfig>([enabled](Config::ThemeConfig &config) {
        config.animations_enabled = enabled;
    });
    if (!result)
    {
        const QString error = tr("无法保存界面动画设置：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存界面动画设置失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

void ThemeSettingsModel::refreshFromConfig()
{
    const Config::ThemeConfig config = Config::value<Config::ThemeConfig>();
    const QString mode = QString::fromStdString(config.mode);
    const QString primaryColor = QColor(QString::fromStdString(config.primary_color)).name(QColor::HexRgb).toUpper();
    bool changed = false;

    if (m_mode != mode)
    {
        m_mode = mode;
        changed = true;
        emit modeChanged();
    }
    if (m_primaryColor != primaryColor)
    {
        m_primaryColor = primaryColor;
        changed = true;
        emit primaryColorChanged();
    }
    if (m_animationsEnabled != config.animations_enabled)
    {
        m_animationsEnabled = config.animations_enabled;
        changed = true;
        emit animationsEnabledChanged();
    }
    if (changed)
    {
        emit settingsChanged();
    }
}
