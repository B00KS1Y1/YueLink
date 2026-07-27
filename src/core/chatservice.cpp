#include "chatservice.h"

#include "ichatrepository.h"
#include "ichattransport.h"
#include "iidentitystore.h"
#include "ipeerdiscovery.h"

#include <QDateTime>
#include <QFileInfo>
#include <QUrl>
#include <QUuid>

#include <spdlog/spdlog.h>

#include <utility>

ChatService::ChatService(std::unique_ptr<IPeerDiscovery> discovery,
                         std::unique_ptr<IChatTransport> transport,
                         std::unique_ptr<IChatRepository> repository,
                         std::unique_ptr<IIdentityStore> identityStore,
                         QObject *parent)
: QObject(parent)
, m_discovery(std::move(discovery))
, m_transport(std::move(transport))
, m_repository(std::move(repository))
, m_identityStore(std::move(identityStore))
{
    Q_ASSERT(m_discovery);
    Q_ASSERT(m_transport);
    Q_ASSERT(m_repository);
    Q_ASSERT(m_identityStore);
    initializeIdentity();
    connectServices();
    initializeRepository();
}

ChatService::~ChatService()
{
    stop();
}

Network::LocalIdentity ChatService::localIdentity() const
{
    return m_identity;
}

QList<Domain::Peer> ChatService::peers() const
{
    return m_peers;
}

bool ChatService::peer(const QString &peerId, Domain::Peer *result) const
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

QList<Domain::Message> ChatService::messages(const QString &peerId, int limit)
{
    loadConversation(peerId);
    const QList<Domain::Message> conversation = m_conversations.value(peerId);
    const int boundedLimit = qBound(1, limit, 5000);
    return conversation.mid(qMax(0, conversation.size() - boundedLimit));
}

int ChatService::onlineCount() const
{
    int count = 0;
    for (const Domain::Peer &peer : m_peers)
    {
        count += peer.online ? 1 : 0;
    }
    return count;
}

int ChatService::totalUnreadCount() const
{
    int count = 0;
    for (const Domain::Peer &peer : m_peers)
    {
        count += peer.unreadCount;
    }
    return count;
}

bool ChatService::running() const
{
    return m_running;
}

QString ChatService::lastError() const
{
    return m_lastError;
}

Domain::OperationResult ChatService::start()
{
    if (m_running)
    {
        return Domain::OperationResult::success();
    }
    if (m_identity.deviceId.isEmpty() || m_identity.displayName.isEmpty())
    {
        const QString error = tr("本机身份信息无效。");
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("identity.invalid"), error);
    }

    spdlog::info("[网络.应用] 正在启动局域网聊天服务");
    if (!m_transport->start(m_identity))
    {
        const QString error = m_transport->lastError();
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("transport.start"), error);
    }
    if (!m_discovery->start(m_identity, m_transport->listeningPort()))
    {
        const QString error = m_discovery->lastError();
        m_transport->stop();
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("discovery.start"), error);
    }

    m_running = true;
    setLastError({});
    emit runningChanged();
    spdlog::info("[网络.应用] 局域网聊天服务已启动 TCP端口={}", m_transport->listeningPort());
    return Domain::OperationResult::success();
}

void ChatService::stop()
{
    if (!m_running)
    {
        return;
    }
    m_discovery->stop();
    m_transport->stop();
    for (Domain::Peer &peer : m_peers)
    {
        peer.online = false;
    }
    m_running = false;
    emit peersChanged();
    emit runningChanged();
    spdlog::info("[网络.应用] 局域网聊天服务已停止");
}

Domain::OperationResult ChatService::markConversationRead(const QString &peerId)
{
    Domain::Peer *peer = mutablePeer(peerId);
    if (!peer)
    {
        return Domain::OperationResult::failure(QStringLiteral("peer.not_found"), tr("找不到指定好友。"));
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
            logRepositoryError("清除未读消息", error);
            return Domain::OperationResult::failure(QStringLiteral("storage.clear_unread"), error);
        }
    }
    peer->unreadCount = 0;
    emit peersChanged();
    return Domain::OperationResult::success();
}

Domain::OperationResult ChatService::updateLocalProfile(const QString &displayName)
{
    const QString normalizedName = displayName.trimmed();
    if (normalizedName.isEmpty() || normalizedName.size() > 64)
    {
        const QString error = tr("昵称需要包含 1 到 64 个字符。");
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("profile.invalid_name"), error);
    }
    if (normalizedName == m_identity.displayName)
    {
        setLastError({});
        return Domain::OperationResult::success();
    }

    Network::LocalIdentity updated = m_identity;
    updated.displayName = normalizedName;
    QString error;
    if (!m_identityStore->save(updated, &error))
    {
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("identity.save"), error);
    }

    m_identity = std::move(updated);
    m_discovery->updateIdentity(m_identity);
    m_transport->updateIdentity(m_identity);
    setLastError({});
    emit localIdentityChanged();
    spdlog::info("[网络.应用] 本机资料已更新");
    return Domain::OperationResult::success();
}

Domain::OperationResult ChatService::sendText(const QString &peerId, const QString &text)
{
    const QString content = text.trimmed();
    if (content.isEmpty())
    {
        return Domain::OperationResult::failure(QStringLiteral("message.empty"), tr("消息不能为空。"));
    }
    if (content.size() > 2000)
    {
        const QString error = tr("消息不能超过 2000 个字符。");
        emit sendFailed(peerId, error);
        return Domain::OperationResult::failure(QStringLiteral("message.too_long"), error);
    }

    Domain::Peer peerRecord;
    if (!m_running || !peer(peerId, &peerRecord) || !peerRecord.online)
    {
        const QString error = tr("好友当前不在线。");
        emit sendFailed(peerId, error);
        return Domain::OperationResult::failure(QStringLiteral("peer.offline"), error);
    }

    Domain::Message message;
    message.messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    message.peerId = peerId;
    message.text = content;
    message.timestamp = QDateTime::currentDateTimeUtc();
    message.deliveryState = Domain::DeliveryState::Sending;
    message.fromMe = true;
    persistMessage(message);
    m_conversations[peerId].append(message);
    updateConversation(peerId, content, message.timestamp, false);
    persistConversation(peerId, content, message.timestamp, false);
    emit conversationChanged(peerId);
    m_transport->sendText(peerRecord.endpoint, message.messageId, content, message.timestamp);
    return Domain::OperationResult::success(message.messageId);
}

Domain::OperationResult ChatService::sendFile(const QString &peerId, const QString &filePath)
{
    Domain::Peer peerRecord;
    if (!m_running || !peer(peerId, &peerRecord) || !peerRecord.online)
    {
        const QString error = tr("好友当前不在线。");
        emit fileTransferFailed(peerId, error, false);
        return Domain::OperationResult::failure(QStringLiteral("peer.offline"), error);
    }

    const QString normalizedPath = QFileInfo(filePath).absoluteFilePath();
    QString error;
    if (!m_transport->sendFile(peerRecord.endpoint, QUrl::fromLocalFile(normalizedPath), &error))
    {
        emit fileTransferFailed(peerId, error, false);
        return Domain::OperationResult::failure(QStringLiteral("file.rejected"), error);
    }
    return Domain::OperationResult::success();
}

int ChatService::sendFiles(const QString &peerId, const QStringList &filePaths)
{
    constexpr qsizetype MaximumBatchSize = 100;
    if (filePaths.size() > MaximumBatchSize)
    {
        emit operationFailed(tr("单次最多发送 %1 个文件。").arg(MaximumBatchSize));
    }

    int accepted = 0;
    const qsizetype count = qMin(filePaths.size(), MaximumBatchSize);
    for (qsizetype index = 0; index < count; ++index)
    {
        accepted += sendFile(peerId, filePaths.at(index)) ? 1 : 0;
    }
    return accepted;
}

Domain::OperationResult ChatService::cancelFileTransfer(const QString &peerId, const QString &transferId)
{
    if (!m_transport->cancelFileTransfer(peerId, transferId))
    {
        const QString error = tr("该文件传输已经结束或不存在。");
        emit fileTransferFailed(peerId, error, false);
        return Domain::OperationResult::failure(QStringLiteral("transfer.not_found"), error);
    }
    return Domain::OperationResult::success();
}

void ChatService::connectServices()
{
    connect(m_discovery.get(), &IPeerDiscovery::peerFound, this, &ChatService::observePeer);
    connect(m_discovery.get(), &IPeerDiscovery::peerLost, this, &ChatService::markPeerOffline);
    connect(m_discovery.get(), &IPeerDiscovery::errorOccurred, this, &ChatService::setLastError);
    connect(m_transport.get(), &IChatTransport::peerObserved, this, [this](const Network::PeerEndpoint &peer) {
        m_discovery->recordPeerActivity(peer.peerId);
        observePeer(peer);
    });
    connect(m_transport.get(), &IChatTransport::textReceived, this, &ChatService::handleTextMessage);
    connect(m_transport.get(), &IChatTransport::textSent, this, [this](const QString &peerId, const QString &messageId) {
        updateMessageState(peerId, messageId, Domain::DeliveryState::Sent);
        persistDeliveryStatus(peerId, messageId, Domain::DeliveryState::Sent);
    });
    connect(m_transport.get(), &IChatTransport::textSendFailed, this, [this](const QString &peerId, const QString &messageId, const QString &reason) {
        updateMessageState(peerId, messageId, Domain::DeliveryState::Failed);
        persistDeliveryStatus(peerId, messageId, Domain::DeliveryState::Failed);
        emit sendFailed(peerId, tr("消息发送失败：%1").arg(reason));
    });
    connect(m_transport.get(), &IChatTransport::fileTransferStarted, this, &ChatService::handleFileTransferStarted);
    connect(m_transport.get(), &IChatTransport::fileTransferProgressed, this, &ChatService::handleFileTransferProgress);
    connect(m_transport.get(), &IChatTransport::fileTransferFinished, this, &ChatService::handleFileTransferResult);
    connect(m_transport.get(), &IChatTransport::errorOccurred, this, &ChatService::setLastError);
}

void ChatService::initializeIdentity()
{
    QString error;
    if (!m_identityStore->load(&m_identity, &error))
    {
        setLastError(error);
        spdlog::error("[身份] 加载本机身份信息失败 原因={}", error.toUtf8().toStdString());
    }
}

void ChatService::initializeRepository()
{
    QString error;
    if (!m_repository->initialize(&error))
    {
        logRepositoryError("初始化", error);
        return;
    }
    m_repositoryReady = true;

    QList<Storage::PeerRecord> records;
    if (!m_repository->loadPeers(&records, &error))
    {
        logRepositoryError("加载好友列表", error);
        return;
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
}

void ChatService::loadConversation(const QString &peerId)
{
    if (!m_repositoryReady || m_loadedConversations.contains(peerId))
    {
        return;
    }

    QString error;
    QList<Storage::MessageRecord> records;
    if (!m_repository->loadMessages(peerId, 500, &records, &error))
    {
        logRepositoryError("加载消息", error);
        return;
    }

    QList<Domain::Message> messages;
    messages.reserve(records.size());
    for (Storage::MessageRecord &record : records)
    {
        Domain::DeliveryState state = Domain::deliveryStateFromName(record.deliveryStatus);
        if (state == Domain::DeliveryState::Sending || state == Domain::DeliveryState::Transferring || state == Domain::DeliveryState::Receiving)
        {
            state = Domain::DeliveryState::Failed;
            if (!m_repository->updateDeliveryStatus(record.peerId, record.messageId, Domain::deliveryStateName(state), &error))
            {
                logRepositoryError("恢复中断消息", error);
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

void ChatService::persistPeer(const Network::PeerEndpoint &peer)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->upsertPeer(peer, &error))
    {
        logRepositoryError("保存好友", error);
    }
}

void ChatService::persistConversation(const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->updateConversation(peerId, lastMessage, timestamp, incrementUnread, &error))
    {
        logRepositoryError("更新会话", error);
    }
}

void ChatService::persistMessage(const Domain::Message &message)
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
        logRepositoryError("保存消息", error);
    }
}

void ChatService::persistDeliveryStatus(const QString &peerId, const QString &messageId, Domain::DeliveryState state)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->updateDeliveryStatus(peerId, messageId, Domain::deliveryStateName(state), &error))
    {
        logRepositoryError("更新送达状态", error);
    }
}

void ChatService::persistFileTransfer(const QString &peerId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->updateFileTransfer(peerId, messageId, progress, Domain::deliveryStateName(state), filePath, &error))
    {
        logRepositoryError("更新文件传输", error);
    }
}

void ChatService::logRepositoryError(const char *operation, const QString &error) const
{
    spdlog::warn("[存储.仓储] 操作失败 操作={} 原因={}", operation, error.toUtf8().toStdString());
}

void ChatService::setLastError(const QString &error)
{
    if (m_lastError == error)
    {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

void ChatService::observePeer(const Network::PeerEndpoint &endpoint)
{
    persistPeer(endpoint);
    const int index = peerIndex(endpoint.peerId);
    if (index < 0)
    {
        Domain::Peer peer;
        peer.endpoint = endpoint;
        peer.lastActivity = QDateTime::currentDateTimeUtc();
        peer.online = true;
        m_peers.append(std::move(peer));
        emit peerDiscovered(endpoint.peerId);
        emit peersChanged();
        return;
    }

    Domain::Peer &peer = m_peers[index];
    const bool changed = peer.endpoint.displayName != endpoint.displayName || peer.endpoint.address != endpoint.address ||
                         peer.endpoint.tcpPort != endpoint.tcpPort || !peer.online;
    peer.endpoint = endpoint;
    peer.online = true;
    if (changed)
    {
        emit peerUpdated(endpoint.peerId);
        emit peersChanged();
    }
}

void ChatService::markPeerOffline(const QString &peerId)
{
    Domain::Peer *peer = mutablePeer(peerId);
    if (!peer || !peer->online)
    {
        return;
    }
    peer->online = false;
    emit peerUpdated(peerId);
    emit peersChanged();
}

void ChatService::handleTextMessage(const Network::TextMessage &message)
{
    observePeer(message.sender);
    Domain::Message received;
    received.messageId = message.messageId;
    received.peerId = message.sender.peerId;
    received.text = message.text;
    received.timestamp = message.timestamp;
    received.deliveryState = Domain::DeliveryState::Received;
    persistMessage(received);
    m_conversations[message.sender.peerId].append(received);
    updateConversation(message.sender.peerId, message.text, message.timestamp, true);
    persistConversation(message.sender.peerId, message.text, message.timestamp, true);
    emit conversationChanged(message.sender.peerId);
    emit messageReceived(message.sender.peerId, message.text);
}

void ChatService::handleFileTransferStarted(const Network::FileTransferInfo &transfer)
{
    observePeer(transfer.peer);
    const bool outgoing = transfer.direction == Network::TransferDirection::Outgoing;
    Domain::Message message;
    message.messageId = transfer.transferId;
    message.peerId = transfer.peer.peerId;
    message.timestamp = transfer.timestamp;
    message.deliveryState = outgoing ? Domain::DeliveryState::Transferring : Domain::DeliveryState::Receiving;
    message.kind = Domain::MessageKind::File;
    message.fileName = transfer.fileName;
    message.filePath = transfer.filePath;
    message.fileSize = transfer.fileSize;
    message.fromMe = outgoing;
    persistMessage(message);
    m_conversations[transfer.peer.peerId].append(message);
    const QString summary = tr("[文件] %1").arg(transfer.fileName);
    updateConversation(transfer.peer.peerId, summary, transfer.timestamp, !outgoing);
    persistConversation(transfer.peer.peerId, summary, transfer.timestamp, !outgoing);
    emit conversationChanged(transfer.peer.peerId);
}

void ChatService::handleFileTransferProgress(const Network::FileTransferProgress &progress)
{
    const Domain::DeliveryState state =
        progress.direction == Network::TransferDirection::Outgoing ? Domain::DeliveryState::Transferring : Domain::DeliveryState::Receiving;
    updateFileTransfer(progress.peerId, progress.transferId, progress.progress, state);
    persistFileTransfer(progress.peerId, progress.transferId, progress.progress, state);
}

void ChatService::handleFileTransferResult(const Network::FileTransferResult &result)
{
    if (result.success)
    {
        const bool outgoing = result.direction == Network::TransferDirection::Outgoing;
        const Domain::DeliveryState state = outgoing ? Domain::DeliveryState::Sent : Domain::DeliveryState::Received;
        updateFileTransfer(result.peerId, result.transferId, 1.0, state, result.filePath);
        persistFileTransfer(result.peerId, result.transferId, 1.0, state, result.filePath);
        if (!outgoing)
        {
            emit fileReceived(result.peerId, result.filePath);
        }
        return;
    }

    const Domain::DeliveryState state = result.cancelled ? Domain::DeliveryState::Cancelled : Domain::DeliveryState::Failed;
    updateMessageState(result.peerId, result.transferId, state);
    persistDeliveryStatus(result.peerId, result.transferId, state);
    if (!result.cancelled)
    {
        const QString reason = result.direction == Network::TransferDirection::Outgoing ? tr("文件发送失败：%1").arg(result.errorMessage)
                                                                                        : tr("文件接收失败：%1").arg(result.errorMessage);
        emit fileTransferFailed(result.peerId, reason, result.direction == Network::TransferDirection::Incoming);
    }
}

void ChatService::updateConversation(const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread)
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
    emit peersChanged();
}

void ChatService::updateMessageState(const QString &peerId, const QString &messageId, Domain::DeliveryState state)
{
    QList<Domain::Message> &messages = m_conversations[peerId];
    for (int index = messages.size() - 1; index >= 0; --index)
    {
        if (messages[index].messageId == messageId)
        {
            messages[index].deliveryState = state;
            emit conversationChanged(peerId);
            return;
        }
    }
}

void ChatService::updateFileTransfer(const QString &peerId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath)
{
    QList<Domain::Message> &messages = m_conversations[peerId];
    for (int index = messages.size() - 1; index >= 0; --index)
    {
        Domain::Message &message = messages[index];
        if (message.messageId != messageId)
        {
            continue;
        }
        message.fileProgress = qBound(0.0, progress, 1.0);
        message.deliveryState = state;
        if (!filePath.isEmpty())
        {
            message.filePath = filePath;
        }
        emit conversationChanged(peerId);
        return;
    }
}

int ChatService::peerIndex(const QString &peerId) const
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

Domain::Peer *ChatService::mutablePeer(const QString &peerId)
{
    const int index = peerIndex(peerId);
    return index < 0 ? nullptr : &m_peers[index];
}
