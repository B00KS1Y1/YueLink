#include "conversationstore.h"

#include "core/ichatrepository.h"

#include <spdlog/spdlog.h>

#include <utility>

ConversationStore::ConversationStore(std::unique_ptr<IChatRepository> repository,
                                     QObject *parent)
: QObject(parent)
, m_repository(std::move(repository))
{
    Q_ASSERT(m_repository);
}

ConversationStore::~ConversationStore() = default;

bool ConversationStore::initialize()
{
    QString error;
    if (!m_repository->initialize(&error))
    {
        reportRepositoryError("初始化", error);
        return false;
    }
    m_repositoryReady = true;

    QList<Storage::PeerRecord> records;
    if (!m_repository->loadPeers(&records, &error))
    {
        reportRepositoryError("加载好友列表", error);
        return false;
    }
    for (const Storage::PeerRecord &record : records)
    {
        Domain::Peer peer;
        peer.endpoint = record.endpoint;
        peer.lastMessage = record.lastMessage;
        peer.lastActivity = record.lastActivity;
        peer.unreadCount = record.unreadCount;
        m_peers.append(std::move(peer));
    }
    return true;
}

QList<Domain::Peer> ConversationStore::peers() const
{
    return m_peers;
}

bool ConversationStore::peer(const QString &peerId, Domain::Peer *result) const
{
    const int index = peerIndex(peerId);
    if (index < 0)
    {
        return false;
    }
    if (result)
    {
        *result = m_peers.at(index);
    }
    return true;
}

QList<Domain::Message> ConversationStore::messages(const QString &peerId, int limit)
{
    loadConversation(peerId);
    const QList<Domain::Message> conversation = m_conversations.value(peerId);
    const int boundedLimit = qBound(1, limit, 5000);
    return conversation.mid(qMax(0, conversation.size() - boundedLimit));
}

int ConversationStore::onlineCount() const
{
    int count = 0;
    for (const Domain::Peer &peer : m_peers)
    {
        count += peer.online ? 1 : 0;
    }
    return count;
}

int ConversationStore::totalUnreadCount() const
{
    int count = 0;
    for (const Domain::Peer &peer : m_peers)
    {
        count += peer.unreadCount;
    }
    return count;
}

void ConversationStore::observePeer(const Network::PeerEndpoint &endpoint)
{
    if (m_repositoryReady)
    {
        QString error;
        if (!m_repository->upsertPeer(endpoint, &error))
        {
            reportRepositoryError("保存好友", error);
        }
    }

    const int index = peerIndex(endpoint.peerId);
    if (index < 0)
    {
        Domain::Peer peer;
        peer.endpoint = endpoint;
        peer.lastActivity = QDateTime::currentDateTimeUtc();
        peer.online = true;
        m_peers.append(std::move(peer));
        emit peersChanged();
        emit peerDiscovered(endpoint.peerId);
        return;
    }

    Domain::Peer &peer = m_peers[index];
    const bool changed = peer.endpoint.displayName != endpoint.displayName
                         || peer.endpoint.address != endpoint.address
                         || peer.endpoint.tcpPort != endpoint.tcpPort
                         || !peer.online;
    peer.endpoint = endpoint;
    peer.online = true;
    if (changed)
    {
        emit peersChanged();
        emit peerUpdated(endpoint.peerId);
    }
}

void ConversationStore::markPeerOffline(const QString &peerId)
{
    Domain::Peer *peer = mutablePeer(peerId);
    if (!peer || !peer->online)
    {
        return;
    }
    peer->online = false;
    emit peersChanged();
    emit peerUpdated(peerId);
}

void ConversationStore::markAllPeersOffline()
{
    bool changed = false;
    for (Domain::Peer &peer : m_peers)
    {
        changed = changed || peer.online;
        peer.online = false;
    }
    if (changed)
    {
        emit peersChanged();
    }
}

Domain::OperationResult ConversationStore::markConversationRead(const QString &peerId)
{
    Domain::Peer *peer = mutablePeer(peerId);
    if (!peer)
    {
        return Domain::OperationResult::failure(QStringLiteral("peer.not_found"),
                                                tr("找不到指定好友。"));
    }
    if (peer->unreadCount == 0)
    {
        return Domain::OperationResult::success();
    }

    if (m_repositoryReady)
    {
        QString error;
        if (!m_repository->clearUnread(peerId, &error))
        {
            reportRepositoryError("清除未读消息", error);
            return Domain::OperationResult::failure(QStringLiteral("storage.clear_unread"),
                                                    error.isEmpty() ? tr("清除未读消息失败。") : error);
        }
    }
    peer->unreadCount = 0;
    emit peersChanged();
    return Domain::OperationResult::success();
}

void ConversationStore::appendMessage(Domain::Message message,
                                      const QString &summary,
                                      bool incrementUnread)
{
    persistMessage(message);
    const QString peerId = message.peerId;
    const QDateTime timestamp = message.timestamp;
    m_conversations[peerId].append(message);
    updateConversation(peerId, summary, timestamp, incrementUnread);
    emit messageAdded(message);
}

void ConversationStore::updateMessageState(const QString &peerId,
                                           const QString &messageId,
                                           Domain::DeliveryState state)
{
    QList<Domain::Message> &messages = m_conversations[peerId];
    for (int index = messages.size() - 1; index >= 0; --index)
    {
        if (messages[index].messageId == messageId)
        {
            messages[index].deliveryState = state;
            break;
        }
    }

    if (m_repositoryReady)
    {
        QString error;
        if (!m_repository->updateDeliveryStatus(peerId,
                                                messageId,
                                                Domain::deliveryStateName(state),
                                                &error))
        {
            reportRepositoryError("更新投递状态", error);
        }
    }
    emit messageStateChanged(peerId, messageId, state);
}

void ConversationStore::updateFileTransfer(const QString &peerId,
                                           const QString &messageId,
                                           qreal progress,
                                           Domain::DeliveryState state,
                                           const QString &filePath)
{
    const qreal boundedProgress = qBound(0.0, progress, 1.0);
    QList<Domain::Message> &messages = m_conversations[peerId];
    for (int index = messages.size() - 1; index >= 0; --index)
    {
        Domain::Message &message = messages[index];
        if (message.messageId != messageId)
        {
            continue;
        }
        message.fileProgress = boundedProgress;
        message.deliveryState = state;
        if (!filePath.isEmpty())
        {
            message.filePath = filePath;
        }
        break;
    }

    if (m_repositoryReady)
    {
        QString error;
        if (!m_repository->updateFileTransfer(peerId,
                                              messageId,
                                              boundedProgress,
                                              Domain::deliveryStateName(state),
                                              filePath,
                                              &error))
        {
            reportRepositoryError("更新文件传输", error);
        }
    }
    emit fileTransferChanged(peerId, messageId, boundedProgress, state, filePath);
}

void ConversationStore::loadConversation(const QString &peerId)
{
    if (!m_repositoryReady || m_loadedConversations.contains(peerId))
    {
        return;
    }

    QString error;
    QList<Storage::MessageRecord> records;
    if (!m_repository->loadMessages(peerId, 500, &records, &error))
    {
        reportRepositoryError("加载消息", error);
        return;
    }

    QList<Domain::Message> messages;
    messages.reserve(records.size());
    for (Storage::MessageRecord &record : records)
    {
        Domain::DeliveryState state = Domain::deliveryStateFromName(record.deliveryStatus);
        if (state == Domain::DeliveryState::Sending
            || state == Domain::DeliveryState::Transferring
            || state == Domain::DeliveryState::Receiving)
        {
            state = Domain::DeliveryState::Failed;
            if (!m_repository->updateDeliveryStatus(record.peerId,
                                                    record.messageId,
                                                    Domain::deliveryStateName(state),
                                                    &error))
            {
                reportRepositoryError("恢复中断消息", error);
            }
        }

        Domain::Message message;
        message.messageId = record.messageId;
        message.peerId = record.peerId;
        message.text = record.text;
        message.timestamp = record.timestamp;
        message.deliveryState = state;
        message.kind = Domain::messageKindFromName(record.messageKind);
        message.fileName = record.fileName;
        message.filePath = record.filePath;
        message.legacyFileSizeText = record.fileSizeText;
        message.fileSize = record.fileSize;
        message.fileProgress = record.fileProgress;
        message.fromMe = record.fromMe;
        messages.append(std::move(message));
    }
    m_conversations.insert(peerId, std::move(messages));
    m_loadedConversations.insert(peerId);
}

void ConversationStore::updateConversation(const QString &peerId,
                                           const QString &lastMessage,
                                           const QDateTime &timestamp,
                                           bool incrementUnread)
{
    const int index = peerIndex(peerId);
    if (index < 0)
    {
        return;
    }
    Domain::Peer peer = m_peers.takeAt(index);
    peer.lastMessage = lastMessage;
    peer.lastActivity = timestamp;
    if (incrementUnread)
    {
        ++peer.unreadCount;
    }
    m_peers.prepend(std::move(peer));

    if (m_repositoryReady)
    {
        QString error;
        if (!m_repository->updateConversation(peerId,
                                              lastMessage,
                                              timestamp,
                                              incrementUnread,
                                              &error))
        {
            reportRepositoryError("更新会话", error);
        }
    }
    emit peersChanged();
}

void ConversationStore::persistMessage(const Domain::Message &message)
{
    if (!m_repositoryReady)
    {
        return;
    }
    Storage::MessageRecord record;
    record.messageId = message.messageId;
    record.peerId = message.peerId;
    record.text = message.text;
    record.timestamp = message.timestamp;
    record.deliveryStatus = Domain::deliveryStateName(message.deliveryState);
    record.messageKind = Domain::messageKindName(message.kind);
    record.fileName = message.fileName;
    record.fileSize = message.fileSize;
    record.fileSizeText = message.legacyFileSizeText;
    record.filePath = message.filePath;
    record.fileProgress = message.fileProgress;
    record.fromMe = message.fromMe;
    QString error;
    if (!m_repository->saveMessage(record, &error))
    {
        reportRepositoryError("保存消息", error);
    }
}

int ConversationStore::peerIndex(const QString &peerId) const
{
    for (int index = 0; index < m_peers.size(); ++index)
    {
        if (m_peers.at(index).endpoint.peerId == peerId)
        {
            return index;
        }
    }
    return -1;
}

Domain::Peer *ConversationStore::mutablePeer(const QString &peerId)
{
    const int index = peerIndex(peerId);
    return index < 0 ? nullptr : &m_peers[index];
}

void ConversationStore::reportRepositoryError(const char *operation,
                                               const QString &error)
{
    const QString reason = error.isEmpty()
                               ? tr("数据仓储操作失败：%1").arg(QString::fromUtf8(operation))
                               : error;
    spdlog::warn("[存储.仓储] 操作失败 操作={} 原因={}",
                 operation,
                 reason.toUtf8().toStdString());
    emit operationFailed(reason);
}
