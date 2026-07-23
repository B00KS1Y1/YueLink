#ifndef DESKTOPNOTIFICATIONSERVICE_H
#define DESKTOPNOTIFICATIONSERVICE_H

#include "inotificationservice.h"

#include <QSystemTrayIcon>

class DesktopNotificationService final : public INotificationService
{
    Q_OBJECT

public:
    explicit DesktopNotificationService(QObject *parent = nullptr);

    void setEnabled(bool enabled) override;
    void showNotification(const QString &title,
                          const QString &message,
                          const QString &contextId) override;

private:
    QSystemTrayIcon m_trayIcon;
    QString m_contextId;
    bool m_enabled = false;
};

#endif // DESKTOPNOTIFICATIONSERVICE_H
