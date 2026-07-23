#ifndef LANCHATMANAGER_H
#define LANCHATMANAGER_H

#include "models/chatmessagemodel.h"
#include "models/peerlistmodel.h"

#include <QAbstractItemModel>
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>

class ChatService;
class IFileLauncher;
class INotificationService;
class QJSEngine;
class QQmlEngine;

namespace Domain
{
struct Message;
}

class LanChatManager final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LanChat)
    QML_SINGLETON
    Q_PROPERTY(QAbstractItemModel *peers READ peers CONSTANT)
    Q_PROPERTY(QAbstractItemModel *messages READ messages CONSTANT)
    Q_PROPERTY(QString localName READ localName NOTIFY localProfileChanged)
    Q_PROPERTY(QString localInitial READ localInitial NOTIFY localProfileChanged)
    Q_PROPERTY(QString currentPeerId READ currentPeerId NOTIFY currentPeerIdChanged)
    Q_PROPERTY(int onlineCount READ onlineCount NOTIFY onlineCountChanged)
    Q_PROPERTY(int totalUnreadCount READ totalUnreadCount NOTIFY totalUnreadCountChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    static void setService(ChatService *service);
    static LanChatManager *create(QQmlEngine *qmlEngine,
                                  QJSEngine *jsEngine);

    explicit LanChatManager(ChatService *service, QObject *parent = nullptr);
    LanChatManager(ChatService *service,
                   std::unique_ptr<IFileLauncher> fileLauncher,
                   std::unique_ptr<INotificationService> notificationService,
                   QObject *parent = nullptr);
    ~LanChatManager() override;

    [[nodiscard]] QAbstractItemModel *peers();
    [[nodiscard]] QAbstractItemModel *messages();
    [[nodiscard]] QString localName() const;
    [[nodiscard]] QString localInitial() const;
    [[nodiscard]] QString currentPeerId() const;
    [[nodiscard]] int onlineCount() const;
    [[nodiscard]] int totalUnreadCount() const;
    [[nodiscard]] bool running() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE bool selectPeer(const QString &peerId);
    Q_INVOKABLE bool markConversationRead(const QString &peerId);
    Q_INVOKABLE QVariantMap peerInfo(const QString &peerId) const;
    Q_INVOKABLE bool updateLocalProfile(const QString &displayName);
    Q_INVOKABLE bool sendMessage(const QString &peerId, const QString &text);
    Q_INVOKABLE bool sendFile(const QString &peerId, const QUrl &fileUrl);
    Q_INVOKABLE int sendFiles(const QString &peerId,
                              const QList<QUrl> &fileUrls);
    Q_INVOKABLE bool cancelFileTransfer(const QString &peerId,
                                        const QString &transferId);
    Q_INVOKABLE bool openFile(const QString &filePath);
    Q_INVOKABLE bool revealFile(const QString &filePath);
    Q_INVOKABLE void setNotificationsEnabled(bool enabled);

signals:
    void localProfileChanged();
    void currentPeerIdChanged();
    void onlineCountChanged();
    void totalUnreadCountChanged();
    void runningChanged();
    void lastErrorChanged();
    void peerDiscovered(const QString &peerId);
    void peerUpdated(const QString &peerId);
    void messageReceived(const QString &peerId, const QString &text);
    void sendFailed(const QString &peerId, const QString &reason);
    void fileReceived(const QString &peerId, const QString &filePath);
    void fileTransferFailed(const QString &peerId, const QString &reason);
    void operationFailed(const QString &reason);
    void notificationActivated(const QString &peerId);

private:
    void connectService();
    void synchronizePeers();
    void synchronizeConversation(const QString &peerId);
    void handleIncomingMessage(const QString &peerId, const QString &text);
    void showIncomingNotification(const QString &peerId,
                                  const QString &message);
    [[nodiscard]] ChatMessageModel::Message toViewMessage(
        const Domain::Message &message) const;
    [[nodiscard]] static QString displayFileSize(qint64 bytes,
                                                 const QString &fallback);
    [[nodiscard]] static QString displayTime(const QDateTime &timestamp);
    [[nodiscard]] static QString initialForName(const QString &name);
    [[nodiscard]] static QString colorForId(const QString &peerId);

    ChatService *m_service = nullptr;
    std::unique_ptr<IFileLauncher> m_fileLauncher;
    std::unique_ptr<INotificationService> m_notificationService;
    PeerListModel m_peerModel;
    ChatMessageModel m_messageModel;
    QString m_currentPeerId;
    static ChatService *s_service;
};

#endif // LANCHATMANAGER_H
