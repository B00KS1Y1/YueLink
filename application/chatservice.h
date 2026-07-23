#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "domain/chattypes.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QStringList>

#include <memory>

class IChatRepository;
class IChatTransport;
class IIdentityStore;
class IPeerDiscovery;

class ChatService final : public QObject
{
    Q_OBJECT

public:
    ChatService(std::unique_ptr<IPeerDiscovery> discovery,
                std::unique_ptr<IChatTransport> transport,
                std::unique_ptr<IChatRepository> repository,
                std::unique_ptr<IIdentityStore> identityStore,
                QObject *parent = nullptr);
    ~ChatService() override;

    [[nodiscard]] Network::LocalIdentity localIdentity() const;
    [[nodiscard]] QList<Domain::Peer> peers() const;
    [[nodiscard]] bool peer(const QString &peerId, Domain::Peer *result) const;
    [[nodiscard]] QList<Domain::Message> messages(const QString &peerId, int limit = 500);
    [[nodiscard]] int onlineCount() const;
    [[nodiscard]] int totalUnreadCount() const;
    [[nodiscard]] bool running() const;
    [[nodiscard]] QString lastError() const;

    [[nodiscard]] Domain::OperationResult start();
    void stop();
    [[nodiscard]] Domain::OperationResult markConversationRead(const QString &peerId);
    [[nodiscard]] Domain::OperationResult updateLocalProfile(const QString &displayName);
    [[nodiscard]] Domain::OperationResult sendText(const QString &peerId, const QString &text);
    [[nodiscard]] Domain::OperationResult sendFile(const QString &peerId, const QString &filePath);
    [[nodiscard]] int sendFiles(const QString &peerId, const QStringList &filePaths);
    [[nodiscard]] Domain::OperationResult cancelFileTransfer(const QString &peerId, const QString &transferId);

signals:
    void localIdentityChanged();
    void peersChanged();
    void conversationChanged(const QString &peerId);
    void runningChanged();
    void lastErrorChanged();
    void peerDiscovered(const QString &peerId);
    void peerUpdated(const QString &peerId);
    void messageReceived(const QString &peerId, const QString &text);
    void sendFailed(const QString &peerId, const QString &reason);
    void fileReceived(const QString &peerId, const QString &filePath);
    void fileTransferFailed(const QString &peerId, const QString &reason, bool incoming);
    void operationFailed(const QString &reason);

private:
    void connectServices();
    void initializeIdentity();
    void initializeRepository();
    void loadConversation(const QString &peerId);
    void persistPeer(const Network::PeerEndpoint &peer);
    void persistConversation(const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread);
    void persistMessage(const Domain::Message &message);
    void persistDeliveryStatus(const QString &peerId, const QString &messageId, Domain::DeliveryState state);
    void persistFileTransfer(const QString &peerId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath = {});
    void logRepositoryError(const char *operation, const QString &error) const;
    void setLastError(const QString &error);
    void observePeer(const Network::PeerEndpoint &peer);
    void markPeerOffline(const QString &peerId);
    void handleTextMessage(const Network::TextMessage &message);
    void handleFileTransferStarted(const Network::FileTransferInfo &transfer);
    void handleFileTransferProgress(const Network::FileTransferProgress &progress);
    void handleFileTransferResult(const Network::FileTransferResult &result);
    void updateConversation(const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread);
    void updateMessageState(const QString &peerId, const QString &messageId, Domain::DeliveryState state);
    void updateFileTransfer(const QString &peerId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath = {});
    [[nodiscard]] int peerIndex(const QString &peerId) const;
    [[nodiscard]] Domain::Peer *mutablePeer(const QString &peerId);

    std::unique_ptr<IPeerDiscovery> m_discovery;
    std::unique_ptr<IChatTransport> m_transport;
    std::unique_ptr<IChatRepository> m_repository;
    std::unique_ptr<IIdentityStore> m_identityStore;
    QList<Domain::Peer> m_peers;
    QHash<QString, QList<Domain::Message>> m_conversations;
    QSet<QString> m_loadedConversations;
    Network::LocalIdentity m_identity;
    QString m_lastError;
    bool m_repositoryReady = false;
    bool m_running = false;
};

#endif // CHATSERVICE_H
