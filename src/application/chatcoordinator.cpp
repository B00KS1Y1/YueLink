#include "chatcoordinator.h"

#include "conversationstore.h"
#include "transfercoordinator.h"
#include "domain/ichatrepository.h"
#include "domain/ichattransport.h"
#include "domain/ipeerdiscovery.h"
#include "config/configapi.h"

#include <QColor>
#include <QDateTime>
#include <QFileInfo>
#include <QImageReader>
#include <QSet>
#include <QSysInfo>
#include <QUuid>

#include <algorithm>
#include <utility>

ChatCoordinator::ChatCoordinator(std::unique_ptr<IPeerDiscovery> discovery,
                                 std::unique_ptr<IChatTransport> transport,
                                 std::unique_ptr<IChatRepository> repository,
                                 QObject *parent)
: QObject(parent)
, m_discovery(std::move(discovery))
, m_transport(std::move(transport))
, m_conversations(std::make_unique<ConversationStore>(std::move(repository)))
{
    Q_ASSERT(m_discovery);
    Q_ASSERT(m_transport);
    Q_ASSERT(m_conversations);
    m_identityReady = initializeIdentity();
    static_cast<void>(m_conversations->initialize());
    m_transfers = std::make_unique<TransferCoordinator>(m_transport.get(),
                                                        m_conversations.get(),
                                                        &m_identity);
    connectServices();
}

ChatCoordinator::~ChatCoordinator()
{
    stop();
}

Network::LocalIdentity ChatCoordinator::localIdentity() const { return m_identity; }
QString ChatCoordinator::localAvatarPath() const { return m_localAvatarPath; }
QString ChatCoordinator::localAvatarColor() const { return m_localAvatarColor; }
QList<Domain::Peer> ChatCoordinator::peers() const { return m_conversations->peers(); }
QList<Domain::Conversation> ChatCoordinator::conversations() const
{
    return m_conversations->conversations();
}
bool ChatCoordinator::peer(const QString &peerId, Domain::Peer *result) const
{
    return m_conversations->peer(peerId, result);
}
bool ChatCoordinator::conversation(const QString &conversationId,
                                   Domain::Conversation *result) const
{
    return m_conversations->conversation(conversationId, result);
}
bool ChatCoordinator::group(const QString &groupId, Domain::Group *result) const
{
    return m_conversations->group(groupId, result);
}
QList<Domain::GroupMember> ChatCoordinator::groupMembers(const QString &groupId) const
{
    return m_conversations->groupMembers(groupId);
}
QList<Domain::Message> ChatCoordinator::messages(const QString &conversationId,
                                                 int limit)
{
    return m_conversations->messages(conversationId, limit);
}
void ChatCoordinator::deliveryCounts(const QString &messageId,
                                     int *deliveredCount,
                                     int *totalCount) const
{
    m_conversations->deliveryCounts(messageId, deliveredCount, totalCount);
}
int ChatCoordinator::onlineCount() const { return m_conversations->onlineCount(); }
int ChatCoordinator::totalUnreadCount() const
{
    return m_conversations->totalUnreadCount();
}
bool ChatCoordinator::running() const { return m_running; }
QString ChatCoordinator::lastError() const { return m_lastError; }

Domain::OperationResult ChatCoordinator::start()
{
    if (m_running)
        return Domain::OperationResult::success();
    if (!m_identityReady || m_identity.deviceId.isEmpty()
        || m_identity.displayName.isEmpty())
    {
        const QString error = m_lastError.isEmpty() ? tr("本机身份信息无效。")
                                                    : m_lastError;
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("identity.invalid"), error);
    }
    if (!m_transport->start(m_identity))
    {
        setLastError(m_transport->lastError());
        return Domain::OperationResult::failure(QStringLiteral("transport.start"),
                                                m_lastError);
    }
    if (!m_discovery->start(m_identity, m_transport->listeningPort()))
    {
        setLastError(m_discovery->lastError());
        m_transport->stop();
        return Domain::OperationResult::failure(QStringLiteral("discovery.start"),
                                                m_lastError);
    }
    m_running = true;
    setLastError({});
    emit runningChanged();
    return Domain::OperationResult::success();
}

void ChatCoordinator::stop()
{
    if (!m_running)
        return;
    m_discovery->stop();
    m_transport->stop();
    m_conversations->markAllPeersOffline();
    m_running = false;
    emit runningChanged();
}

Domain::OperationResult ChatCoordinator::refreshPeerDiscovery()
{
    if (!m_running)
        return Domain::OperationResult::failure(QStringLiteral("discovery.not_running"),
                                                tr("局域网服务未启动。"));
    m_discovery->probe();
    return Domain::OperationResult::success();
}

Domain::OperationResult ChatCoordinator::markConversationRead(
    const QString &conversationId)
{
    return m_conversations->markConversationRead(conversationId);
}

Domain::OperationResult ChatCoordinator::createGroup(
    const QString &name,
    const QStringList &memberIds)
{
    const QString normalizedName = name.trimmed();
    if (normalizedName.isEmpty() || normalizedName.size() > 64)
        return Domain::OperationResult::failure(QStringLiteral("group.invalid_name"),
                                                tr("群名称需要包含 1 到 64 个字符。"));

    QSet<QString> uniqueIds;
    for (const QString &memberId : memberIds)
    {
        if (!memberId.isEmpty() && memberId != m_identity.deviceId)
            uniqueIds.insert(memberId);
    }
    if (uniqueIds.size() < 2 || uniqueIds.size() > 31)
        return Domain::OperationResult::failure(QStringLiteral("group.invalid_members"),
                                                tr("请选择 2 到 31 位联系人。"));

    Domain::Group group;
    group.groupId = QStringLiteral("group:%1")
                        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    group.name = normalizedName;
    group.ownerId = m_identity.deviceId;
    group.createdAt = QDateTime::currentDateTimeUtc();
    group.members.append({m_identity.deviceId,
                          m_identity.displayName,
                          Domain::GroupRole::Owner});
    for (const QString &memberId : uniqueIds)
    {
        Domain::Peer peerRecord;
        if (!m_conversations->peer(memberId, &peerRecord))
            return Domain::OperationResult::failure(QStringLiteral("group.unknown_member"),
                                                    tr("部分联系人已经不可用。"));
        group.members.append({memberId,
                              peerRecord.endpoint.displayName,
                              Domain::GroupRole::Member});
    }
    if (!m_conversations->upsertGroup(group))
        return Domain::OperationResult::failure(QStringLiteral("group.save"),
                                                tr("无法保存群聊。"));

    const Network::GroupSnapshot snapshot = networkSnapshot(group);
    for (const QString &memberId : uniqueIds)
    {
        Domain::Peer peerRecord;
        if (m_running && m_conversations->peer(memberId, &peerRecord)
            && peerRecord.online)
            m_transport->sendGroupSnapshot(peerRecord.endpoint, snapshot);
    }
    return Domain::OperationResult::success(group.groupId);
}

Domain::OperationResult ChatCoordinator::updateLocalProfile(
    const QString &displayName,
    const QString &avatarPath,
    const QString &avatarColor)
{
    const QString normalizedName = displayName.trimmed();
    if (normalizedName.isEmpty() || normalizedName.size() > 64)
        return Domain::OperationResult::failure(QStringLiteral("profile.invalid_name"),
                                                tr("昵称需要包含 1 到 64 个字符。"));
    QString normalizedAvatarPath = avatarPath.trimmed();
    if (!normalizedAvatarPath.isEmpty())
    {
        const QFileInfo info(normalizedAvatarPath);
        if (!info.isFile() || QImageReader::imageFormat(info.absoluteFilePath()).isEmpty())
            return Domain::OperationResult::failure(QStringLiteral("profile.invalid_avatar"),
                                                    tr("头像图片无效或不可读取。"));
        normalizedAvatarPath = info.absoluteFilePath();
    }
    const QColor parsedColor(avatarColor.trimmed());
    if (!parsedColor.isValid())
        return Domain::OperationResult::failure(QStringLiteral("profile.invalid_color"),
                                                tr("头像颜色无效。"));
    const QString normalizedColor = parsedColor.name(QColor::HexRgb);

    Config::IdentityConfig updated = Config::value<Config::IdentityConfig>();
    updated.device_id = m_identity.deviceId.toStdString();
    updated.display_name = normalizedName.toStdString();
    updated.avatar_path = normalizedAvatarPath.toStdString();
    updated.avatar_color = normalizedColor.toStdString();
    const Config::Result result = Config::set(std::move(updated));
    if (!result)
        return Domain::OperationResult::failure(QStringLiteral("identity.save"),
                                                result.errorMessage);

    const QString previousName = m_identity.displayName;
    m_identity.displayName = normalizedName;
    m_localAvatarPath = normalizedAvatarPath;
    m_localAvatarColor = normalizedColor;
    m_discovery->updateIdentity(m_identity);
    m_transport->updateIdentity(m_identity);
    if (previousName != normalizedName)
    {
        for (const Domain::Conversation &conversation : m_conversations->conversations())
        {
            if (conversation.kind != Domain::ConversationKind::Group)
                continue;
            Domain::Group group;
            if (!m_conversations->group(conversation.conversationId, &group)
                || group.ownerId != m_identity.deviceId)
                continue;
            for (Domain::GroupMember &member : group.members)
            {
                if (member.peerId == m_identity.deviceId)
                {
                    member.displayName = normalizedName;
                    break;
                }
            }
            ++group.revision;
            if (!m_conversations->upsertGroup(group))
                continue;
            const Network::GroupSnapshot snapshot = networkSnapshot(group);
            for (const Domain::GroupMember &member : group.members)
            {
                Domain::Peer peerRecord;
                if (member.peerId != m_identity.deviceId && m_running
                    && m_conversations->peer(member.peerId, &peerRecord)
                    && peerRecord.online)
                    m_transport->sendGroupSnapshot(peerRecord.endpoint, snapshot);
            }
        }
    }
    emit localIdentityChanged();
    return Domain::OperationResult::success();
}

Domain::OperationResult ChatCoordinator::sendText(const QString &conversationId,
                                                  const QString &text)
{
    const QString content = text.trimmed();
    if (content.isEmpty() || content.size() > 2000)
        return Domain::OperationResult::failure(QStringLiteral("message.invalid"),
                                                tr("消息需要包含 1 到 2000 个字符。"));

    Domain::Conversation conversation;
    if (!m_conversations->conversation(conversationId, &conversation))
        return Domain::OperationResult::failure(QStringLiteral("conversation.unknown"),
                                                tr("会话不存在。"));

    Domain::Group group;
    if (conversation.kind == Domain::ConversationKind::Group)
    {
        if (!m_conversations->group(conversationId, &group))
            return Domain::OperationResult::failure(QStringLiteral("group.unknown"),
                                                    tr("群聊不存在。"));
        bool localMember = false;
        for (const Domain::GroupMember &member : group.members)
            localMember = localMember || member.peerId == m_identity.deviceId;
        if (!localMember)
            return Domain::OperationResult::failure(QStringLiteral("group.not_member"),
                                                    tr("你已不在该群聊中。"));
    }

    Domain::Message message;
    message.messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    message.conversationId = conversationId;
    message.senderId = m_identity.deviceId;
    message.text = content;
    message.timestamp = QDateTime::currentDateTimeUtc();
    message.deliveryState = Domain::DeliveryState::Sending;
    if (!m_conversations->appendMessage(message, content, false))
        return Domain::OperationResult::failure(QStringLiteral("message.save"),
                                                tr("无法保存待发送消息。"));

    if (conversation.kind == Domain::ConversationKind::Direct)
    {
        Domain::Peer peerRecord;
        if (!resolveOnlineDirectPeer(conversationId, &peerRecord, false))
        {
            m_conversations->updateMessageState(conversationId,
                                                message.messageId,
                                                Domain::DeliveryState::Failed);
            return Domain::OperationResult::failure(QStringLiteral("peer.offline"),
                                                    tr("联系人当前不在线。"));
        }
        m_transport->sendText(peerRecord.endpoint,
                              message.messageId,
                              {},
                              content,
                              message.timestamp);
        return Domain::OperationResult::success(message.messageId);
    }

    for (const Domain::GroupMember &member : group.members)
    {
        if (member.peerId == m_identity.deviceId)
            continue;
        Domain::MessageDelivery delivery;
        delivery.messageId = message.messageId;
        delivery.conversationId = conversationId;
        delivery.recipientId = member.peerId;
        m_conversations->saveDelivery(std::move(delivery));
    }
    for (const Domain::GroupMember &member : group.members)
    {
        if (member.peerId != m_identity.deviceId)
            dispatchPendingToPeer(member.peerId);
    }
    return Domain::OperationResult::success(message.messageId);
}

Domain::OperationResult ChatCoordinator::sendFile(const QString &conversationId,
                                                  const QString &filePath)
{
    Domain::Peer peerRecord;
    if (!resolveOnlineDirectPeer(conversationId, &peerRecord, true))
        return Domain::OperationResult::failure(QStringLiteral("file.unsupported"),
                                                tr("群聊暂不支持文件，或联系人当前离线。"));
    return m_transfers->sendFile(peerRecord, filePath);
}

int ChatCoordinator::sendFiles(const QString &conversationId,
                               const QStringList &filePaths)
{
    Domain::Peer peerRecord;
    return resolveOnlineDirectPeer(conversationId, &peerRecord, true)
               ? m_transfers->sendFiles(peerRecord, filePaths)
               : 0;
}

Domain::OperationResult ChatCoordinator::cancelFileTransfer(
    const QString &conversationId,
    const QString &transferId)
{
    const QString peerId = Domain::peerIdFromDirectConversation(conversationId);
    if (peerId.isEmpty())
        return Domain::OperationResult::failure(QStringLiteral("file.group_unsupported"),
                                                tr("群聊暂不支持文件传输。"));
    return m_transfers->cancel(peerId, transferId);
}

void ChatCoordinator::connectServices()
{
    connect(m_discovery.get(), &IPeerDiscovery::peerFound, this,
            [this](const Network::PeerEndpoint &endpoint) {
                Domain::Peer previous;
                const bool known = m_conversations->peer(endpoint.peerId, &previous);
                const bool shouldSynchronizeGroups =
                    !known || !previous.online
                    || previous.endpoint.displayName != endpoint.displayName
                    || previous.endpoint.address != endpoint.address
                    || previous.endpoint.tcpPort != endpoint.tcpPort;
                m_conversations->observePeer(endpoint);
                Domain::Peer peerRecord;
                if (shouldSynchronizeGroups
                    && m_conversations->peer(endpoint.peerId, &peerRecord))
                    synchronizeOwnedGroupsWithPeer(peerRecord);
                dispatchPendingToPeer(endpoint.peerId);
            });
    connect(m_discovery.get(), &IPeerDiscovery::peerLost,
            m_conversations.get(), &ConversationStore::markPeerOffline);
    connect(m_discovery.get(), &IPeerDiscovery::errorOccurred,
            this, &ChatCoordinator::setLastError);
    connect(m_transport.get(), &IChatTransport::peerObserved, this,
            [this](const Network::PeerEndpoint &endpoint) {
                m_discovery->recordPeerActivity(endpoint.peerId);
                Domain::Peer previous;
                const bool known = m_conversations->peer(endpoint.peerId, &previous);
                const bool shouldSynchronizeGroups =
                    !known || !previous.online
                    || previous.endpoint.displayName != endpoint.displayName
                    || previous.endpoint.address != endpoint.address
                    || previous.endpoint.tcpPort != endpoint.tcpPort;
                m_conversations->observePeer(endpoint);
                Domain::Peer peerRecord;
                if (shouldSynchronizeGroups
                    && m_conversations->peer(endpoint.peerId, &peerRecord))
                    synchronizeOwnedGroupsWithPeer(peerRecord);
                dispatchPendingToPeer(endpoint.peerId);
            });
    connect(m_transport.get(), &IChatTransport::textReceived,
            this, &ChatCoordinator::handleTextMessage);
    connect(m_transport.get(), &IChatTransport::groupSnapshotReceived,
            this, &ChatCoordinator::handleGroupSnapshot);
    connect(m_transport.get(), &IChatTransport::textSent, this,
            [this](const QString &peerId, const QString &messageId) {
                Domain::Message message;
                if (!m_conversations->message(messageId, &message))
                    return;
                Domain::Conversation conversation;
                if (m_conversations->conversation(message.conversationId, &conversation)
                    && conversation.kind == Domain::ConversationKind::Group)
                {
                    Domain::MessageDelivery delivery;
                    delivery.messageId = messageId;
                    delivery.conversationId = message.conversationId;
                    delivery.recipientId = peerId;
                    delivery.state = Domain::DeliveryState::Sent;
                    m_conversations->saveDelivery(std::move(delivery));
                }
                else
                {
                    m_conversations->updateMessageState(message.conversationId,
                                                        messageId,
                                                        Domain::DeliveryState::Sent);
                }
            });
    connect(m_transport.get(), &IChatTransport::textSendFailed, this,
            [this](const QString &peerId, const QString &messageId, const QString &reason) {
                Domain::Message message;
                if (!m_conversations->message(messageId, &message))
                    return;
                Domain::Conversation conversation;
                if (m_conversations->conversation(message.conversationId, &conversation)
                    && conversation.kind == Domain::ConversationKind::Group)
                {
                    Domain::MessageDelivery delivery;
                    delivery.messageId = messageId;
                    delivery.conversationId = message.conversationId;
                    delivery.recipientId = peerId;
                    delivery.state = Domain::DeliveryState::Pending;
                    delivery.errorMessage = reason;
                    m_conversations->saveDelivery(std::move(delivery));
                }
                else
                {
                    m_conversations->updateMessageState(message.conversationId,
                                                        messageId,
                                                        Domain::DeliveryState::Failed);
                    emit sendFailed(message.conversationId,
                                    tr("消息发送失败：%1").arg(reason));
                }
            });
    connect(m_transport.get(), &IChatTransport::errorOccurred,
            this, &ChatCoordinator::setLastError);

    connect(m_conversations.get(), &ConversationStore::peersChanged,
            this, &ChatCoordinator::peersChanged);
    connect(m_conversations.get(), &ConversationStore::conversationsChanged,
            this, &ChatCoordinator::conversationsChanged);
    connect(m_conversations.get(), &ConversationStore::peerDiscovered,
            this, &ChatCoordinator::peerDiscovered);
    connect(m_conversations.get(), &ConversationStore::peerUpdated,
            this, &ChatCoordinator::peerUpdated);
    connect(m_conversations.get(), &ConversationStore::groupChanged,
            this, &ChatCoordinator::groupChanged);
    connect(m_conversations.get(), &ConversationStore::messageAdded,
            this, &ChatCoordinator::messageAdded);
    connect(m_conversations.get(), &ConversationStore::messageStateChanged,
            this, &ChatCoordinator::messageStateChanged);
    connect(m_conversations.get(), &ConversationStore::deliveryChanged,
            this, &ChatCoordinator::deliveryChanged);
    connect(m_conversations.get(), &ConversationStore::fileTransferChanged,
            this, &ChatCoordinator::fileTransferChanged);
    connect(m_conversations.get(), &ConversationStore::operationFailed,
            this, &ChatCoordinator::operationFailed);

    connect(m_transfers.get(), &TransferCoordinator::fileReceived, this,
            [this](const QString &peerId, const QString &path) {
                emit fileReceived(Domain::directConversationId(peerId), path);
            });
    connect(m_transfers.get(), &TransferCoordinator::fileTransferFailed, this,
            [this](const QString &peerId, const QString &reason, bool incoming) {
                emit fileTransferFailed(Domain::directConversationId(peerId),
                                        reason,
                                        incoming);
            });
    connect(m_transfers.get(), &TransferCoordinator::operationFailed,
            this, &ChatCoordinator::operationFailed);
}

bool ChatCoordinator::initializeIdentity()
{
    const Config::Result loadResult = Config::reload<Config::IdentityConfig>();
    if (!loadResult)
    {
        setLastError(loadResult.errorMessage);
        return false;
    }
    Config::IdentityConfig config = Config::value<Config::IdentityConfig>();
    QString deviceId = QString::fromStdString(config.device_id).trimmed();
    QString displayName = QString::fromStdString(config.display_name).trimmed();
    QString avatarPath = QString::fromStdString(config.avatar_path).trimmed();
    QString avatarColor = QString::fromStdString(config.avatar_color).trimmed();
    bool changed = false;
    if (deviceId.isEmpty())
    {
        deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        changed = true;
    }
    if (displayName.isEmpty())
    {
        displayName = QSysInfo::machineHostName().trimmed();
        if (displayName.isEmpty())
            displayName = tr("YueLink 用户");
        changed = true;
    }
    displayName = displayName.left(64);
    if (!avatarPath.isEmpty())
    {
        const QFileInfo info(avatarPath);
        if (!info.isFile() || QImageReader::imageFormat(info.absoluteFilePath()).isEmpty())
        {
            avatarPath.clear();
            changed = true;
        }
        else
            avatarPath = info.absoluteFilePath();
    }
    const QColor parsedColor(avatarColor);
    if (!parsedColor.isValid())
    {
        avatarColor = QStringLiteral("#4f7cff");
        changed = true;
    }
    else
        avatarColor = parsedColor.name(QColor::HexRgb);

    m_identity = {deviceId, displayName};
    m_localAvatarPath = avatarPath;
    m_localAvatarColor = avatarColor;
    if (!changed)
        return true;
    config.device_id = deviceId.toStdString();
    config.display_name = displayName.toStdString();
    config.avatar_path = avatarPath.toStdString();
    config.avatar_color = avatarColor.toStdString();
    const Config::Result saveResult = Config::set(std::move(config));
    if (!saveResult)
    {
        setLastError(saveResult.errorMessage);
        return false;
    }
    return true;
}

void ChatCoordinator::handleTextMessage(const Network::TextMessage &message)
{
    m_conversations->observePeer(message.sender);
    QString conversationId;
    if (message.groupId.isEmpty())
    {
        conversationId = Domain::directConversationId(message.sender.peerId);
    }
    else
    {
        Domain::Group group;
        if (!m_conversations->group(message.groupId, &group))
        {
            constexpr qsizetype MaximumPendingGroupMessages = 256;
            if (m_pendingGroupMessages.size() >= MaximumPendingGroupMessages)
                m_pendingGroupMessages.erase(m_pendingGroupMessages.begin());
            m_pendingGroupMessages.insert(message.groupId, message);
            return;
        }
        bool senderIsMember = false;
        bool localIsMember = false;
        for (const Domain::GroupMember &member : group.members)
        {
            senderIsMember = senderIsMember || member.peerId == message.sender.peerId;
            localIsMember = localIsMember || member.peerId == m_identity.deviceId;
        }
        if (!senderIsMember || !localIsMember)
            return;
        conversationId = message.groupId;
    }

    Domain::Message received;
    received.messageId = message.messageId;
    received.conversationId = conversationId;
    received.senderId = message.sender.peerId;
    received.text = message.text;
    received.timestamp = message.timestamp;
    received.deliveryState = Domain::DeliveryState::Received;
    if (m_conversations->appendMessage(std::move(received), message.text, true))
        emit messageReceived(conversationId, message.text);
}

void ChatCoordinator::handleGroupSnapshot(const Network::GroupSnapshot &snapshot)
{
    Domain::Group current;
    if (m_conversations->group(snapshot.groupId, &current))
    {
        if (current.ownerId != snapshot.ownerId
            || current.revision >= snapshot.revision)
            return;
    }
    Domain::Group group;
    group.groupId = snapshot.groupId;
    group.name = snapshot.name;
    group.ownerId = snapshot.ownerId;
    group.revision = snapshot.revision;
    group.createdAt = snapshot.createdAt;
    for (const Network::GroupMemberInfo &member : snapshot.members)
    {
        group.members.append({member.peerId,
                              member.displayName,
                              member.owner ? Domain::GroupRole::Owner
                                           : Domain::GroupRole::Member});
    }
    if (!m_conversations->upsertGroup(group))
        return;

    QList<Network::TextMessage> pending =
        m_pendingGroupMessages.values(snapshot.groupId);
    m_pendingGroupMessages.remove(snapshot.groupId);
    std::sort(pending.begin(), pending.end(),
              [](const Network::TextMessage &left,
                 const Network::TextMessage &right) {
                  return left.timestamp < right.timestamp;
              });
    for (const Network::TextMessage &message : pending)
        handleTextMessage(message);
}

void ChatCoordinator::synchronizeOwnedGroupsWithPeer(const Domain::Peer &peer)
{
    if (!m_running || !peer.online)
        return;
    for (const Domain::Conversation &conversation : m_conversations->conversations())
    {
        if (conversation.kind != Domain::ConversationKind::Group)
            continue;
        Domain::Group group;
        if (!m_conversations->group(conversation.conversationId, &group)
            || group.ownerId != m_identity.deviceId)
            continue;
        bool targetIsMember = false;
        bool memberNameChanged = false;
        for (Domain::GroupMember &member : group.members)
        {
            if (member.peerId == peer.endpoint.peerId)
            {
                targetIsMember = true;
                if (member.displayName != peer.endpoint.displayName)
                {
                    member.displayName = peer.endpoint.displayName;
                    memberNameChanged = true;
                }
                break;
            }
        }
        if (!targetIsMember)
            continue;
        if (!memberNameChanged)
        {
            m_transport->sendGroupSnapshot(peer.endpoint, networkSnapshot(group));
            continue;
        }

        ++group.revision;
        if (!m_conversations->upsertGroup(group))
            continue;
        const Network::GroupSnapshot snapshot = networkSnapshot(group);
        for (const Domain::GroupMember &member : group.members)
        {
            if (member.peerId == m_identity.deviceId)
                continue;
            Domain::Peer recipient;
            if (m_conversations->peer(member.peerId, &recipient) && recipient.online)
                m_transport->sendGroupSnapshot(recipient.endpoint, snapshot);
        }
    }
}

void ChatCoordinator::dispatchPendingToPeer(const QString &peerId)
{
    Domain::Peer peerRecord;
    if (!m_running || !m_conversations->peer(peerId, &peerRecord) || !peerRecord.online)
        return;
    const QList<Domain::MessageDelivery> deliveries =
        m_conversations->pendingDeliveriesForPeer(peerId);
    for (Domain::MessageDelivery delivery : deliveries)
    {
        Domain::Message message;
        if (!m_conversations->message(delivery.messageId, &message))
            continue;
        delivery.state = Domain::DeliveryState::Sending;
        delivery.errorMessage.clear();
        m_conversations->saveDelivery(delivery);
        m_transport->sendText(peerRecord.endpoint,
                              message.messageId,
                              message.conversationId,
                              message.text,
                              message.timestamp);
    }
}

Network::GroupSnapshot ChatCoordinator::networkSnapshot(const Domain::Group &group) const
{
    Network::GroupSnapshot snapshot;
    snapshot.groupId = group.groupId;
    snapshot.name = group.name;
    snapshot.ownerId = group.ownerId;
    snapshot.revision = group.revision;
    snapshot.createdAt = group.createdAt;
    snapshot.sender = Network::PeerEndpoint{m_identity.deviceId,
                                            m_identity.displayName,
                                            {},
                                            0};
    for (const Domain::GroupMember &member : group.members)
    {
        snapshot.members.append({member.peerId,
                                 member.displayName,
                                 member.role == Domain::GroupRole::Owner});
    }
    return snapshot;
}

void ChatCoordinator::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

bool ChatCoordinator::resolveOnlineDirectPeer(const QString &conversationId,
                                              Domain::Peer *peerRecord,
                                              bool fileOperation)
{
    Domain::Conversation conversation;
    Domain::Peer peer;
    if (m_running && m_conversations->conversation(conversationId, &conversation)
        && conversation.kind == Domain::ConversationKind::Direct
        && m_conversations->peer(conversation.peerId, &peer) && peer.online)
    {
        if (peerRecord)
            *peerRecord = peer;
        return true;
    }
    const QString error = tr("该操作仅支持在线联系人的直接会话。");
    if (fileOperation)
        emit fileTransferFailed(conversationId, error, false);
    else
        emit sendFailed(conversationId, error);
    return false;
}
