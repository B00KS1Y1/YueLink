/**
 * @file inotificationservice.h
 * @brief 声明桌面通知抽象接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef INOTIFICATIONSERVICE_H
#define INOTIFICATIONSERVICE_H

#include <QObject>
#include <QString>

class INotificationService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造桌面通知服务。
     * @param parent 可选的 QObject 父对象。
     */
    explicit INotificationService(QObject *parent = nullptr)
    : QObject(parent)
    {
    }

    /** @brief 销毁桌面通知服务。 */
    ~INotificationService() override = default;

    /**
     * @brief 启用或禁用桌面通知。
     * @param enabled 是否允许显示通知。
     */
    virtual void setEnabled(bool enabled) = 0;
    /**
     * @brief 显示桌面通知。
     * @param title 通知标题。
     * @param message 通知正文。
     * @param contextId 激活通知时返回的上下文标识。
     */
    virtual void showNotification(const QString &title,
                                  const QString &message,
                                  const QString &contextId) = 0;

signals:
    /**
     * @brief 用户激活通知时发出。
     * @param contextId 通知关联的上下文标识。
     */
    void notificationActivated(const QString &contextId);
};

#endif // INOTIFICATIONSERVICE_H
