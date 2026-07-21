#ifndef LANCHATMANAGER_H
#define LANCHATMANAGER_H

#include "domain/networktypes.h"
#include "models/chatmessagemodel.h"
#include "models/peerlistmodel.h"

#include <QAbstractItemModel>
#include <QList>
#include <QObject>
#include <QSet>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>

class IChatTransport;
class IChatRepository;
class IFileLauncher;
class INotificationService;
class IPeerDiscovery;

class LanChatManager : public QObject
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
    explicit LanChatManager(QObject *parent = nullptr);
    LanChatManager(std::unique_ptr<IPeerDiscovery> discovery,
                   std::unique_ptr<IChatTransport> transport,
                   QObject *parent = nullptr);
    LanChatManager(std::unique_ptr<IPeerDiscovery> discovery,
                   std::unique_ptr<IChatTransport> transport,
                   std::unique_ptr<IChatRepository> repository,
                   QObject *parent = nullptr);
    LanChatManager(std::unique_ptr<IPeerDiscovery> discovery,
                   std::unique_ptr<IChatTransport> transport,
                   std::unique_ptr<IChatRepository> repository,
                   std::unique_ptr<IFileLauncher> fileLauncher,
                   QObject *parent = nullptr);
    LanChatManager(std::unique_ptr<IPeerDiscovery> discovery,
                   std::unique_ptr<IChatTransport> transport,
                   std::unique_ptr<IChatRepository> repository,
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
    void connectServices();
    void loadIdentity();
    void initializeRepository();
    void loadConversation(const QString &peerId);
    void persistPeer(const Network::PeerEndpoint &peer);
    void persistConversation(const QString &peerId,
                             const QString &lastMessage,
                             const QDateTime &timestamp,
                             bool incrementUnread);
    void persistMessage(const QString &peerId,
                        const ChatMessageModel::Message &message,
                        const QDateTime &timestamp);
    void persistDeliveryStatus(const QString &peerId,
                               const QString &messageId,
                               const QString &status);
    void persistFileTransfer(const QString &peerId,
                             const QString &messageId,
                             qreal progress,
                             const QString &status,
                             const QString &filePath = {});
    void logRepositoryError(const char *operation, const QString &error) const;
    void setLastError(const QString &error);
    void observePeer(const Network::PeerEndpoint &peer);
    void markPeerOffline(const QString &peerId);
    void handleTextMessage(const Network::TextMessage &message);
    void handleFileTransferStarted(const Network::FileTransferInfo &transfer);
    void handleFileTransferProgress(const Network::FileTransferProgress &progress);
    void handleFileTransferResult(const Network::FileTransferResult &result);
    [[nodiscard]] bool shouldMarkIncomingUnread(const QString &peerId) const;
    void showIncomingNotification(const QString &peerId,
                                  const QString &message);
    [[nodiscard]] static QString displayFileSize(qint64 bytes);
    [[nodiscard]] static QString displayTime(const QDateTime &timestamp);

    std::unique_ptr<IPeerDiscovery> m_discovery;
    std::unique_ptr<IChatTransport> m_transport;
    std::unique_ptr<IChatRepository> m_repository;
    std::unique_ptr<IFileLauncher> m_fileLauncher;
    std::unique_ptr<INotificationService> m_notificationService;
    PeerListModel m_peerModel;
    ChatMessageModel m_messageModel;
    QSet<QString> m_loadedConversations;
    Network::LocalIdentity m_identity;
    QString m_localInitial;
    QString m_currentPeerId;
    QString m_lastError;
    bool m_repositoryReady = false;
    bool m_running = false;
};

#endif // LANCHATMANAGER_H
