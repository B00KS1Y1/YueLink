/**
 * @file appsettings.cpp
 * @brief 实现面向 QML 的分类设置组合入口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#include "appsettings.h"

AppSettings::AppSettings(QObject *parent)
: QObject(parent)
, m_application(new ApplicationSettingsModel(this))
, m_theme(new ThemeSettingsModel(this))
, m_log(new LogSettingsModel(this))
{
}

ApplicationSettingsModel *AppSettings::application() const
{
    return m_application;
}

ThemeSettingsModel *AppSettings::theme() const
{
    return m_theme;
}

LogSettingsModel *AppSettings::log() const
{
    return m_log;
}
