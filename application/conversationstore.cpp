#include "conversationstore.h"

#include "domain/ichatrepository.h"

#include <QDateTime>

#include <QyLog.h>

#include <utility>

ConversationStore::ConversationStore(std::unique_ptr<IChatRepository> repository, QObject *parent)
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
        reportRepositoryError("initialize", error);
        return false;
    }
    m_repositoryReady = true;

    QList<Domain::Group> groups;
    QList<Domain::MessageDelivery> deliveries;
    if (!m_repository->loadPeers(&m_peers, &error) || !m_repository->loadConversations(&m_conversationList, &error) ||
        !m_repository->loadGroups(&groups, &error) || !m_repository->loadDeliveries(&deliveries, &error))
    {
        reportRepositoryError("restore", error);
        return false;
    }
    for (Domain::Group &group : groups)
    {
        m_groups.insert(group.groupId, std::move(group));
    }
    for (Domain::MessageDelivery &delivery : deliveries)
    {
        if (delivery.state != Domain::DeliveryState::Sent)
        {
            delivery.state = Domain::DeliveryState::Pending;
        }
        m_deliveries[delivery.messageId].insert(delivery.recipientId, delivery);
        if (delivery.state == Domain::DeliveryState::Pending)
        {
            static_cast<void>(m_repository->saveDelivery(delivery, &error));
        }
    }
    return true;
}

QList<Domain::Peer> ConversationStore::peers() const
{
    return m_peers;
}

QList<Domain::Conversation> ConversationStore::conversations() const
{
    return m_conversationList;
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

bool ConversationStore::conversation(const QString &conversationId, Domain::Conversation *result) const
{
    const int index = conversationIndex(conversationId);
    if (index < 0)
    {
        return false;
    }
    if (result)
    {
        *result = m_conversationList.at(index);
    }
    return true;
}

bool ConversationStore::group(const QString &groupId, Domain::Group *result) const
{
    const auto iterator = m_groups.constFind(groupId);
    if (iterator == m_groups.cend())
    {
        return false;
    }
    if (result)
    {
        *result = iterator.value();
    }
    return true;
}

QList<Domain::GroupMember> ConversationStore::groupMembers(const QString &groupId) const
{
    const auto iterator = m_groups.constFind(groupId);
    return iterator == m_groups.cend() ? QList<Domain::GroupMember>{} : iterator->members;
}

QList<Domain::Message> ConversationStore::messages(const QString &conversationId, int limit)
{
    if (!m_loadedConversations.contains(conversationId))
    {
        loadConversation(conversationId);
    }
    const QList<Domain::Message> values = m_messages.value(conversationId);
    const int boundedLimit = qBound(1, limit, 5000);
    return values.size() <= boundedLimit ? values : values.sliced(values.size() - boundedLimit);
}

bool ConversationStore::message(const QString &messageId, Domain::Message *result)
{
    for (const QList<Domain::Message> &conversationMessages : std::as_const(m_messages))
    {
        for (const Domain::Message &candidate : conversationMessages)
        {
            if (candidate.metadata.messageId == messageId)
            {
                if (result)
                {
                    *result = candidate;
                }
                return true;
            }
        }
    }
    if (!m_repositoryReady)
    {
        return false;
    }
    QString error;
    Domain::Message stored;
    if (!m_repository->loadMessage(messageId, &stored, &error))
    {
        if (!error.isEmpty())
        {
            reportRepositoryError("loadMessage", error);
        }
        return false;
    }
    if (result)
    {
        *result = stored;
    }
    return true;
}

QList<Domain::MessageDelivery> ConversationStore::pendingDeliveriesForPeer(const QString &peerId) const
{
    QList<Domain::MessageDelivery> result;
    for (const auto &byRecipient : m_deliveries)
    {
        const auto iterator = byRecipient.constFind(peerId);
        if (iterator != byRecipient.cend() && iterator->state == Domain::DeliveryState::Pending)
        {
            result.append(iterator.value());
        }
    }
    return result;
}

void ConversationStore::deliveryCounts(const QString &messageId, int *deliveredCount, int *totalCount) const
{
    const auto iterator = m_deliveries.constFind(messageId);
    int delivered = 0;
    int total = 0;
    if (iterator != m_deliveries.cend())
    {
        total = iterator->size();
        for (const Domain::MessageDelivery &delivery : iterator.value())
        {
            delivered += delivery.state == Domain::DeliveryState::Sent ? 1 : 0;
        }
    }
    if (deliveredCount)
    {
        *deliveredCount = delivered;
    }
    if (totalCount)
    {
        *totalCount = total;
    }
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
    for (const Domain::Conversation &conversation : m_conversationList)
    {
        count += conversation.unreadCount;
    }
    return count;
}

void ConversationStore::observePeer(const Network::PeerEndpoint &endpoint)
{
    if (!endpoint.isValid())
    {
        return;
    }
    const int existingIndex = peerIndex(endpoint.peerId);
    const bool discovered = existingIndex < 0;
    if (discovered)
    {
        Domain::Peer peer;
        peer.endpoint = endpoint;
        peer.online = true;
        m_peers.append(peer);
    }
    else
    {
        const Domain::Peer &existing = m_peers.at(existingIndex);
        const bool changed = !existing.online || existing.endpoint.displayName != endpoint.displayName || existing.endpoint.address != endpoint.address ||
                             existing.endpoint.tcpPort != endpoint.tcpPort;
        if (!changed)
        {
            return;
        }
        m_peers[existingIndex].endpoint = endpoint;
        m_peers[existingIndex].online = true;
    }

    QString error;
    const Domain::Peer &peerRecord = m_peers.at(peerIndex(endpoint.peerId));
    if (m_repositoryReady && !m_repository->upsertPeer(peerRecord, &error))
    {
        reportRepositoryError("upsertPeer", error);
    }

    const QString directId = Domain::directConversationId(endpoint.peerId);
    Domain::Conversation *direct = mutableConversation(directId);
    if (!direct)
    {
        Domain::Conversation conversation;
        conversation.conversationId = directId;
        conversation.kind = Domain::ConversationKind::Direct;
        conversation.peerId = endpoint.peerId;
        conversation.title = endpoint.displayName;
        conversation.lastActivity = QDateTime::currentDateTimeUtc();
        conversation.memberCount = 2;
        m_conversationList.append(conversation);
        if (m_repositoryReady && !m_repository->saveConversation(conversation, &error))
        {
            reportRepositoryError("saveConversation", error);
        }
        emit conversationsChanged();
    }
    else if (direct->title != endpoint.displayName)
    {
        direct->title = endpoint.displayName;
        if (m_repositoryReady && !m_repository->saveConversation(*direct, &error))
        {
            reportRepositoryError("saveConversation", error);
        }
        emit conversationsChanged();
    }

    emit peersChanged();
    if (discovered)
    {
        emit peerDiscovered(endpoint.peerId);
    }
    else
    {
        emit peerUpdated(endpoint.peerId);
    }
}

void ConversationStore::markPeerOffline(const QString &peerId)
{
    Domain::Peer *record = mutablePeer(peerId);
    if (!record || !record->online)
    {
        return;
    }
    record->online = false;
    emit peersChanged();
    emit peerUpdated(peerId);
    emit conversationsChanged();
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
        emit conversationsChanged();
    }
}

bool ConversationStore::upsertGroup(const Domain::Group &group)
{
    if (!group.groupId.startsWith(QLatin1String("group:")) || group.name.isEmpty() || group.ownerId.isEmpty() || group.members.size() < 2 ||
        group.members.size() > 32)
    {
        return false;
    }
    QSet<QString> memberIds;
    int ownerCount = 0;
    for (const Domain::GroupMember &member : group.members)
    {
        if (member.peerId.isEmpty() || member.displayName.trimmed().isEmpty() || memberIds.contains(member.peerId) ||
            (member.role == Domain::GroupRole::Owner && member.peerId != group.ownerId))
        {
            return false;
        }
        memberIds.insert(member.peerId);
        ownerCount += member.role == Domain::GroupRole::Owner ? 1 : 0;
    }
    if (ownerCount != 1 || !memberIds.contains(group.ownerId))
    {
        return false;
    }
    const auto current = m_groups.constFind(group.groupId);
    if (current != m_groups.cend() && current->revision >= group.revision)
    {
        return true;
    }

    Domain::Conversation conversation;
    if (!this->conversation(group.groupId, &conversation))
    {
        conversation.conversationId = group.groupId;
        conversation.kind = Domain::ConversationKind::Group;
        conversation.lastActivity = group.createdAt;
    }
    conversation.title = group.name;
    conversation.memberCount = group.members.size();

    QString error;
    if (m_repositoryReady && (!m_repository->saveConversation(conversation, &error) || !m_repository->saveGroup(group, &error)))
    {
        reportRepositoryError("saveGroup", error);
        return false;
    }
    const int index = conversationIndex(group.groupId);
    if (index < 0)
    {
        m_conversationList.append(conversation);
    }
    else
    {
        m_conversationList[index] = conversation;
    }
    m_groups.insert(group.groupId, group);
    emit conversationsChanged();
    emit groupChanged(group.groupId);
    return true;
}

Domain::OperationResult ConversationStore::markConversationRead(const QString &conversationId)
{
    Domain::Conversation *record = mutableConversation(conversationId);
    if (!record)
    {
        return Domain::OperationResult::failure(QStringLiteral("conversation.unknown"), tr("会话不存在。"));
    }
    if (record->unreadCount == 0)
    {
        return Domain::OperationResult::success();
    }
    QString error;
    if (m_repositoryReady && !m_repository->clearUnread(conversationId, &error))
    {
        reportRepositoryError("clearUnread", error);
        return Domain::OperationResult::failure(QStringLiteral("storage.clear_unread"), error);
    }
    record->unreadCount = 0;
    emit conversationsChanged();
    return Domain::OperationResult::success();
}

Domain::OperationResult ConversationStore::restoreConversation(const QString &conversationId)
{
    Domain::Conversation *record = mutableConversation(conversationId);
    if (!record)
    {
        return Domain::OperationResult::failure(QStringLiteral("conversation.unknown"), tr("会话不存在。"));
    }
    if (!record->hidden)
    {
        return Domain::OperationResult::success();
    }

    Domain::Conversation restored = *record;
    restored.hidden = false;
    restored.lastActivity = QDateTime::currentDateTimeUtc();
    QString error;
    if (m_repositoryReady && !m_repository->saveConversation(restored, &error))
    {
        reportRepositoryError("restoreConversation", error);
        return Domain::OperationResult::failure(QStringLiteral("storage.restore_conversation"), error);
    }
    *record = std::move(restored);
    emit conversationsChanged();
    return Domain::OperationResult::success();
}

Domain::OperationResult ConversationStore::setConversationPinned(const QString &conversationId, bool pinned)
{
    Domain::Conversation *record = mutableConversation(conversationId);
    if (!record || record->hidden)
    {
        return Domain::OperationResult::failure(QStringLiteral("conversation.unknown"), tr("会话不存在。"));
    }
    if (record->pinned == pinned)
    {
        return Domain::OperationResult::success();
    }

    QString error;
    if (m_repositoryReady && !m_repository->setConversationPinned(conversationId, pinned, &error))
    {
        reportRepositoryError("setConversationPinned", error);
        return Domain::OperationResult::failure(QStringLiteral("storage.pin_conversation"), error);
    }
    record->pinned = pinned;
    emit conversationsChanged();
    return Domain::OperationResult::success();
}

Domain::OperationResult ConversationStore::removeConversation(const QString &conversationId)
{
    Domain::Conversation *record = mutableConversation(conversationId);
    if (!record || record->hidden)
    {
        return Domain::OperationResult::failure(QStringLiteral("conversation.unknown"), tr("会话不存在。"));
    }

    const QList<Domain::Message> conversationMessages = messages(conversationId, 5000);
    for (const Domain::Message &message : conversationMessages)
    {
        const bool activeTransfer = Domain::messageAttachment(message) &&
                                    (message.deliveryState == Domain::DeliveryState::Pending ||
                                     message.deliveryState == Domain::DeliveryState::Sending ||
                                     message.deliveryState == Domain::DeliveryState::AwaitingAcceptance ||
                                     message.deliveryState == Domain::DeliveryState::Transferring ||
                                     message.deliveryState == Domain::DeliveryState::Receiving);
        if (activeTransfer)
        {
            return Domain::OperationResult::failure(QStringLiteral("conversation.active_transfer"),
                                                    tr("当前会话仍有文件正在传输，请完成或取消传输后再删除。"));
        }
    }

    QString error;
    if (m_repositoryReady && !m_repository->removeConversation(conversationId, &error))
    {
        reportRepositoryError("removeConversation", error);
        return Domain::OperationResult::failure(QStringLiteral("storage.remove_conversation"), error);
    }

    record->lastMessage.clear();
    record->unreadCount = 0;
    record->pinned = false;
    record->hidden = true;
    m_messages.remove(conversationId);
    m_loadedConversations.remove(conversationId);
    auto deliveryIterator = m_deliveries.begin();
    while (deliveryIterator != m_deliveries.end())
    {
        bool belongsToConversation = false;
        for (const Domain::MessageDelivery &delivery : deliveryIterator.value())
        {
            if (delivery.conversationId == conversationId)
            {
                belongsToConversation = true;
                break;
            }
        }
        if (belongsToConversation)
        {
            deliveryIterator = m_deliveries.erase(deliveryIterator);
        }
        else
        {
            ++deliveryIterator;
        }
    }

    emit conversationRemoved(conversationId);
    emit conversationsChanged();
    return Domain::OperationResult::success();
}

bool ConversationStore::appendMessage(Domain::Message message, const QString &summary, bool incrementUnread)
{
    Domain::Message duplicate;
    if (this->message(message.metadata.messageId, &duplicate))
    {
        return false;
    }
    if (!mutableConversation(message.metadata.conversationId))
    {
        return false;
    }

    QString error;
    if (m_repositoryReady && !m_repository->saveMessage(message, &error))
    {
        reportRepositoryError("saveMessage", error);
        return false;
    }
    m_loadedConversations.insert(message.metadata.conversationId);
    m_messages[message.metadata.conversationId].append(message);
    updateConversationSummary(message.metadata.conversationId, summary, message.metadata.timestamp, incrementUnread);
    emit messageAdded(message);
    return true;
}

void ConversationStore::updateMessageState(const QString &conversationId, const QString &messageId, Domain::DeliveryState state)
{
    if (!m_loadedConversations.contains(conversationId))
    {
        loadConversation(conversationId);
    }
    QList<Domain::Message> &values = m_messages[conversationId];
    for (Domain::Message &message : values)
    {
        if (message.metadata.messageId != messageId || message.deliveryState == state)
        {
            continue;
        }
        message.deliveryState = state;
        QString error;
        if (m_repositoryReady && !m_repository->updateMessageState(conversationId, messageId, state, &error))
        {
            reportRepositoryError("updateMessageState", error);
        }
        emit messageStateChanged(conversationId, messageId, state);
        return;
    }
}

void ConversationStore::updateFileTransfer(
    const QString &conversationId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath)
{
    if (!m_loadedConversations.contains(conversationId))
    {
        loadConversation(conversationId);
    }
    QList<Domain::Message> &values = m_messages[conversationId];
    for (Domain::Message &message : values)
    {
        if (message.metadata.messageId != messageId)
        {
            continue;
        }
        message.localAttachment.progress = qBound(0.0, progress, 1.0);
        message.deliveryState = state;
        if (!filePath.isEmpty())
        {
            message.localAttachment.filePath = filePath;
        }
        QString error;
        if (m_repositoryReady && !m_repository->updateFileTransfer(conversationId, messageId, message.localAttachment.progress, state, filePath, &error))
        {
            reportRepositoryError("updateFileTransfer", error);
        }
        emit fileTransferChanged(conversationId, messageId, message.localAttachment.progress, state, message.localAttachment.filePath);
        return;
    }
}

void ConversationStore::saveDelivery(Domain::MessageDelivery delivery)
{
    delivery.lastAttempt = QDateTime::currentDateTimeUtc();
    m_deliveries[delivery.messageId].insert(delivery.recipientId, delivery);
    QString error;
    if (m_repositoryReady && !m_repository->saveDelivery(delivery, &error))
    {
        reportRepositoryError("saveDelivery", error);
    }
    recomputeAggregateState(delivery.conversationId, delivery.messageId);
}

void ConversationStore::loadConversation(const QString &conversationId)
{
    m_loadedConversations.insert(conversationId);
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    QList<Domain::Message> values;
    if (!m_repository->loadMessages(conversationId, 500, &values, &error))
    {
        reportRepositoryError("loadMessages", error);
        return;
    }
    m_messages.insert(conversationId, std::move(values));
}

void ConversationStore::updateConversationSummary(const QString &conversationId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread)
{
    Domain::Conversation *record = mutableConversation(conversationId);
    if (!record)
    {
        return;
    }
    record->lastMessage = lastMessage;
    record->lastActivity = timestamp;
    record->hidden = false;
    if (incrementUnread)
    {
        ++record->unreadCount;
    }
    QString error;
    if (m_repositoryReady && !m_repository->updateConversation(conversationId, lastMessage, timestamp, incrementUnread, &error))
    {
        reportRepositoryError("updateConversation", error);
    }
    emit conversationsChanged();
}

void ConversationStore::recomputeAggregateState(const QString &conversationId, const QString &messageId)
{
    int delivered = 0;
    int total = 0;
    deliveryCounts(messageId, &delivered, &total);
    if (total > 0)
    {
        const Domain::DeliveryState state = delivered == total ? Domain::DeliveryState::Sent
                                            : delivered > 0    ? Domain::DeliveryState::Partial
                                                               : Domain::DeliveryState::Sending;
        updateMessageState(conversationId, messageId, state);
    }
    emit deliveryChanged(conversationId, messageId, delivered, total);
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

int ConversationStore::conversationIndex(const QString &conversationId) const
{
    for (int index = 0; index < m_conversationList.size(); ++index)
    {
        if (m_conversationList.at(index).conversationId == conversationId)
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

Domain::Conversation *ConversationStore::mutableConversation(const QString &conversationId)
{
    const int index = conversationIndex(conversationId);
    return index < 0 ? nullptr : &m_conversationList[index];
}

void ConversationStore::reportRepositoryError(const char *operation, const QString &error)
{
    const QString reason = error.isEmpty() ? tr("聊天数据操作失败。") : tr("聊天数据操作失败：%1").arg(error);
    QLOG_ERROR() << QStringLiteral("[存储.会话] 操作失败 操作=") << operation << QStringLiteral("原因=") << reason;
    emit operationFailed(reason);
}
