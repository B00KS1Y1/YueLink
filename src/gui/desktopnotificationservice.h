/**
 * @file desktopnotificationservice.h
 * @brief 声明系统托盘通知适配器。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef DESKTOPNOTIFICATIONSERVICE_H
#define DESKTOPNOTIFICATIONSERVICE_H

#include "inotificationservice.h"

#include <QSystemTrayIcon>

class DesktopNotificationService final : public INotificationService
{
    Q_OBJECT

public:
    /**
     * @brief 构造系统托盘通知服务。
     * @param parent 可选的 QObject 父对象。
     */
    explicit DesktopNotificationService(QObject *parent = nullptr);

    /**
     * @brief 启用或禁用桌面通知。
     * @param enabled 是否允许显示通知。
     */
    void setEnabled(bool enabled) override;
    /**
     * @brief 显示桌面通知。
     * @param title 通知标题。
     * @param message 通知正文。
     * @param contextId 激活通知时返回的上下文标识。
     */
    void showNotification(const QString &title,
                          const QString &message,
                          const QString &contextId) override;

private:
    QSystemTrayIcon m_trayIcon;
    QString m_contextId;
    bool m_enabled = false;
};

#endif // DESKTOPNOTIFICATIONSERVICE_H
