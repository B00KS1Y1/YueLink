#include "desktopnotificationservice.h"

#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QStyle>

#include <spdlog/spdlog.h>

DesktopNotificationService::DesktopNotificationService(QObject *parent)
: INotificationService(parent)
{
    QIcon icon = QGuiApplication::windowIcon();
    if (icon.isNull())
    {
        icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    }
    m_trayIcon.setIcon(icon);
    m_trayIcon.setToolTip(tr("YueLink"));
    connect(&m_trayIcon,
            &QSystemTrayIcon::messageClicked,
            this,
            [this]() {
                if (!m_contextId.isEmpty())
                {
                    emit notificationActivated(m_contextId);
                }
            });
}

void DesktopNotificationService::setEnabled(bool enabled)
{
    m_enabled = enabled;
    const bool available = QSystemTrayIcon::isSystemTrayAvailable();
    m_trayIcon.setVisible(enabled && available);
    if (enabled && !available)
    {
        spdlog::warn("[platform.notification] system tray is unavailable");
    }
    else
    {
        spdlog::info("[platform.notification] notifications enabled={}",
                     enabled);
    }
}

void DesktopNotificationService::showNotification(const QString &title,
                                                   const QString &message,
                                                   const QString &contextId)
{
    if (!m_enabled || !m_trayIcon.isVisible())
    {
        return;
    }

    m_contextId = contextId;
    m_trayIcon.showMessage(title,
                           message,
                           QSystemTrayIcon::Information,
                           6000);
    spdlog::debug("[platform.notification] notification shown context_id={} title={}",
                  contextId.toUtf8().toStdString(),
                  title.toUtf8().toStdString());
}
