/**
 * @file settingsmodelbase.cpp
 * @brief 实现设置分类模型共享的自动保存状态管理。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#include "settingsmodelbase.h"

namespace
{
constexpr int SavedStateDurationMs = 1500;
} // namespace

SettingsModelBase::SettingsModelBase(QObject *parent)
: QObject(parent)
{
    m_savedStateTimer.setSingleShot(true);
    m_savedStateTimer.setInterval(SavedStateDurationMs);
    connect(&m_savedStateTimer, &QTimer::timeout, this, [this]() {
        setSaveState(Idle);
    });
}

SettingsModelBase::SaveState SettingsModelBase::saveState() const
{
    return m_saveState;
}

QString SettingsModelBase::errorMessage() const
{
    return m_errorMessage;
}

void SettingsModelBase::markSaved()
{
    setErrorMessage({});
    setSaveState(Saved);
    m_savedStateTimer.start();
}

void SettingsModelBase::markError(const QString &error)
{
    m_savedStateTimer.stop();
    setErrorMessage(error);
    setSaveState(Error);
}

void SettingsModelBase::setSaveState(SaveState state)
{
    if (m_saveState == state)
    {
        return;
    }
    m_saveState = state;
    emit saveStateChanged();
}

void SettingsModelBase::setErrorMessage(const QString &error)
{
    if (m_errorMessage == error)
    {
        return;
    }
    m_errorMessage = error;
    emit errorMessageChanged();
}
