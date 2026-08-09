#include "desktopnotificationservice.h"

#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QStyle>

#include <QsLog.h>

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
        QLOG_WARN() << QStringLiteral("[平台.通知] 系统托盘不可用");
    }
    else
    {
        QLOG_INFO() << QStringLiteral("[平台.通知] 通知启用状态=") << enabled;
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
    QLOG_DEBUG() << QStringLiteral("[平台.通知] 通知已显示 上下文标识=") << contextId
                           << QStringLiteral("标题=") << title;
}
