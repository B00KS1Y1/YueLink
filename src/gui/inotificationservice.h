#ifndef INOTIFICATIONSERVICE_H
#define INOTIFICATIONSERVICE_H

#include <QObject>
#include <QString>

class INotificationService : public QObject
{
    Q_OBJECT

public:
    explicit INotificationService(QObject *parent = nullptr)
    : QObject(parent)
    {
    }

    ~INotificationService() override = default;

    virtual void setEnabled(bool enabled) = 0;
    virtual void showNotification(const QString &title,
                                  const QString &message,
                                  const QString &contextId) = 0;

signals:
    void notificationActivated(const QString &contextId);
};

#endif // INOTIFICATIONSERVICE_H
