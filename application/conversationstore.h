/**
 * @file conversationstore.h
 * @brief 声明联系人、统一会话、群组、消息与投递缓存。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef CONVERSATIONSTORE_H
#define CONVERSATIONSTORE_H

#include "domain/chattypes.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>

#include <memory>

class IChatRepository;

class ConversationStore final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造会话存储并接管数据仓储所有权。
     * @param repository 数据仓储；其生命周期由本对象管理。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ConversationStore(std::unique_ptr<IChatRepository> repository, QObject *parent = nullptr);
    /** @brief 销毁会话存储及其拥有的数据仓储。 */
    ~ConversationStore() override;

    /**
     * @brief 初始化仓储并恢复全部摘要、群组及待发送投递。
     * @return 初始化成功时返回 @c true。
     */
    [[nodiscard]] bool initialize();
    /**
     * @brief 返回全部联系人。
     * @return 联系人列表副本。
     */
    [[nodiscard]] QList<Domain::Peer> peers() const;
    /**
     * @brief 返回全部统一会话。
     * @return 会话列表副本。
     */
    [[nodiscard]] QList<Domain::Conversation> conversations() const;
    /**
     * @brief 查找联系人。
     * @param peerId 联系人设备标识。
     * @param[out] result 找到时接收联系人；允许为空。
     * @return 联系人存在时返回 @c true。
     */
    [[nodiscard]] bool peer(const QString &peerId, Domain::Peer *result) const;
    /**
     * @brief 查找会话。
     * @param conversationId 会话标识。
     * @param[out] result 找到时接收会话；允许为空。
     * @return 会话存在时返回 @c true。
     */
    [[nodiscard]] bool conversation(const QString &conversationId, Domain::Conversation *result) const;
    /**
     * @brief 查找群组。
     * @param groupId 群组标识。
     * @param[out] result 找到时接收群组；允许为空。
     * @return 群组存在时返回 @c true。
     */
    [[nodiscard]] bool group(const QString &groupId, Domain::Group *result) const;
    /**
     * @brief 返回指定群组成员。
     * @param groupId 群组标识。
     * @return 群组成员；群组未知时返回空列表。
     */
    [[nodiscard]] QList<Domain::GroupMember> groupMembers(const QString &groupId) const;
    /**
     * @brief 返回指定会话的最近消息并按需加载缓存。
     * @param conversationId 会话标识。
     * @param limit 最多返回的消息数量，限制在 1 到 5000。
     * @return 按本地插入顺序排列的消息。
     */
    [[nodiscard]] QList<Domain::Message> messages(const QString &conversationId, int limit = 500);
    /**
     * @brief 根据消息标识查找消息并按需访问仓储。
     * @param messageId 消息标识。
     * @param[out] result 找到时接收消息；允许为空。
     * @return 消息存在时返回 @c true。
     */
    [[nodiscard]] bool message(const QString &messageId, Domain::Message *result);
    /**
     * @brief 返回指定联系人的待投递消息。
     * @param peerId 接收方设备标识。
     * @return 当前处于等待状态的投递记录。
     */
    [[nodiscard]] QList<Domain::MessageDelivery> pendingDeliveriesForPeer(const QString &peerId) const;
    /**
     * @brief 返回消息的已发送数与总接收方数。
     * @param messageId 消息标识。
     * @param[out] deliveredCount 接收已发送数；允许为空。
     * @param[out] totalCount 接收总数；允许为空。
     */
    void deliveryCounts(const QString &messageId, int *deliveredCount, int *totalCount) const;
    /**
     * @brief 返回当前在线联系人数量。
     * @return 在线联系人数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回全部会话未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;

    /**
     * @brief 新增或刷新联系人并确保其直接会话存在。
     * @param endpoint 已观察到的网络端点。
     */
    void observePeer(const Network::PeerEndpoint &endpoint);
    /**
     * @brief 将联系人标记为离线。
     * @param peerId 联系人设备标识。
     */
    void markPeerOffline(const QString &peerId);
    /** @brief 将全部联系人标记为离线。 */
    void markAllPeersOffline();
    /**
     * @brief 新增或应用更高修订号的群组快照。
     * @param group 群组快照。
     * @return 保存成功或旧快照被忽略时返回 @c true。
     */
    [[nodiscard]] bool upsertGroup(const Domain::Group &group);
    /**
     * @brief 将指定会话标记为已读。
     * @param conversationId 会话标识。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult markConversationRead(const QString &conversationId);
    /**
     * @brief 恢复隐藏会话并将其作为最近打开的会话。
     * @param conversationId 会话标识。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult restoreConversation(const QString &conversationId);
    /**
     * @brief 更新会话置顶状态并持久化。
     * @param conversationId 会话标识。
     * @param pinned 是否置顶。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult setConversationPinned(const QString &conversationId, bool pinned);
    /**
     * @brief 删除会话本地消息并从消息列表隐藏。
     * @param conversationId 会话标识。
     * @return 结构化操作结果。
     *
     * 联系人、群组元数据与本地附件文件会保留；存在活动附件传输时拒绝删除。
     */
    [[nodiscard]] Domain::OperationResult removeConversation(const QString &conversationId);
    /**
     * @brief 幂等追加消息并更新会话摘要。
     * @param message 待追加消息。
     * @param summary 会话列表摘要。
     * @param incrementUnread 是否增加未读数。
     * @return 新消息成功保存时返回 @c true；重复或失败时返回 @c false。
     */
    [[nodiscard]] bool appendMessage(Domain::Message message, const QString &summary, bool incrementUnread);
    /**
     * @brief 更新消息状态并持久化。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param state 新状态。
     */
    void updateMessageState(const QString &conversationId, const QString &messageId, Domain::DeliveryState state);
    /**
     * @brief 更新文件传输消息。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param progress 传输进度。
     * @param state 新状态。
     * @param filePath 可用时提供本地路径。
     */
    void updateFileTransfer(const QString &conversationId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath = {});
    /**
     * @brief 新增或更新逐成员投递并重算聚合状态。
     * @param delivery 投递记录。
     */
    void saveDelivery(Domain::MessageDelivery delivery);

signals:
    /** @brief 联系人集合或状态发生变化时发出。 */
    void peersChanged();
    /** @brief 会话集合或摘要发生变化时发出。 */
    void conversationsChanged();
    /**
     * @brief 会话已删除本地消息并隐藏时发出。
     * @param conversationId 已隐藏的会话标识。
     */
    void conversationRemoved(const QString &conversationId);
    /**
     * @brief 发现此前未知的联系人时发出。
     * @param peerId 联系人设备标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 联系人状态发生变化时发出。
     * @param peerId 联系人设备标识。
     */
    void peerUpdated(const QString &peerId);
    /**
     * @brief 群组创建或更新时发出。
     * @param groupId 群组标识。
     */
    void groupChanged(const QString &groupId);
    /**
     * @brief 新消息加入会话时发出。
     * @param message 新消息。
     */
    void messageAdded(const Domain::Message &message);
    /**
     * @brief 消息状态发生变化时发出。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param state 新状态。
     */
    void messageStateChanged(const QString &conversationId, const QString &messageId, Domain::DeliveryState state);
    /**
     * @brief 逐成员投递统计发生变化时发出。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param deliveredCount 已发送数量。
     * @param totalCount 接收方总数。
     */
    void deliveryChanged(const QString &conversationId, const QString &messageId, int deliveredCount, int totalCount);
    /**
     * @brief 文件传输消息发生变化时发出。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param progress 传输进度。
     * @param state 新状态。
     * @param filePath 可用时为本地路径。
     */
    void fileTransferChanged(const QString &conversationId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath);
    /**
     * @brief 仓储操作失败时发出。
     * @param reason 用户可读的失败原因。
     */
    void operationFailed(const QString &reason);

private:
    /**
     * @brief 将指定会话加载到内存缓存。
     * @param conversationId 会话标识。
     */
    void loadConversation(const QString &conversationId);
    /**
     * @brief 更新会话摘要并持久化。
     * @param conversationId 会话标识。
     * @param lastMessage 最新摘要。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读数。
     */
    void updateConversationSummary(const QString &conversationId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread);
    /**
     * @brief 根据逐成员记录重算群消息聚合状态。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     */
    void recomputeAggregateState(const QString &conversationId, const QString &messageId);
    /**
     * @brief 查找联系人列表索引。
     * @param peerId 联系人设备标识。
     * @return 索引；不存在时返回 @c -1。
     */
    [[nodiscard]] int peerIndex(const QString &peerId) const;
    /**
     * @brief 查找会话列表索引。
     * @param conversationId 会话标识。
     * @return 索引；不存在时返回 @c -1。
     */
    [[nodiscard]] int conversationIndex(const QString &conversationId) const;
    /**
     * @brief 返回可修改联系人。
     * @param peerId 联系人设备标识。
     * @return 联系人指针；不存在时返回 @c nullptr。
     */
    [[nodiscard]] Domain::Peer *mutablePeer(const QString &peerId);
    /**
     * @brief 返回可修改会话。
     * @param conversationId 会话标识。
     * @return 会话指针；不存在时返回 @c nullptr。
     */
    [[nodiscard]] Domain::Conversation *mutableConversation(const QString &conversationId);
    /**
     * @brief 记录并广播仓储错误。
     * @param operation 失败操作名称。
     * @param error 仓储错误文本。
     */
    void reportRepositoryError(const char *operation, const QString &error);

    std::unique_ptr<IChatRepository> m_repository;
    QList<Domain::Peer> m_peers;
    QList<Domain::Conversation> m_conversationList;
    QHash<QString, Domain::Group> m_groups;
    QHash<QString, QList<Domain::Message>> m_messages;
    QHash<QString, QHash<QString, Domain::MessageDelivery>> m_deliveries;
    QSet<QString> m_loadedConversations;
    bool m_repositoryReady = false;
};

#endif // CONVERSATIONSTORE_H
