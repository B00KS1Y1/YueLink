/**
 * @file conversationstore.h
 * @brief 声明负责会话缓存与持久化的应用层存储。
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
     * @param repository 会话数据仓储；其生命周期由本对象管理。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ConversationStore(std::unique_ptr<IChatRepository> repository,
                               QObject *parent = nullptr);
    /** @brief 销毁会话存储及其拥有的数据仓储。 */
    ~ConversationStore() override;

    /**
     * @brief 初始化仓储并恢复已持久化的节点摘要。
     * @return 仓储可用且节点恢复成功时返回 @c true。
     */
    [[nodiscard]] bool initialize();
    /**
     * @brief 返回全部已知节点。
     * @return 已知节点列表的副本。
     */
    [[nodiscard]] QList<Domain::Peer> peers() const;
    /**
     * @brief 根据标识查找节点。
     * @param peerId 待查找的节点标识。
     * @param[out] result 找到时接收节点信息；允许传入 @c nullptr。
     * @return 节点存在时返回 @c true。
     */
    [[nodiscard]] bool peer(const QString &peerId, Domain::Peer *result) const;
    /**
     * @brief 返回指定会话的最近消息并按需加载缓存。
     * @param peerId 会话对应的节点标识。
     * @param limit 最多返回的消息数量，取值会限制在 1 到 5000。
     * @return 按时间顺序排列的最近消息。
     */
    [[nodiscard]] QList<Domain::Message> messages(const QString &peerId,
                                                  int limit = 500);
    /**
     * @brief 返回当前在线节点数量。
     * @return 在线节点数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有会话的未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;

    /**
     * @brief 新增或刷新观察到的节点并持久化其端点。
     * @param endpoint 已观察到的节点端点。
     */
    void observePeer(const Network::PeerEndpoint &endpoint);
    /**
     * @brief 将已知节点标记为离线。
     * @param peerId 已不可用节点的标识。
     */
    void markPeerOffline(const QString &peerId);
    /** @brief 将全部已知节点标记为离线并合并发送一次模型更新。 */
    void markAllPeersOffline();
    /**
     * @brief 将指定会话标记为已读并持久化。
     * @param peerId 会话对应的节点标识。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult markConversationRead(const QString &peerId);
    /**
     * @brief 追加消息并更新对应会话摘要。
     * @param message 待追加的领域消息。
     * @param summary 会话列表使用的最新消息摘要。
     * @param incrementUnread 是否增加会话未读计数。
     */
    void appendMessage(Domain::Message message,
                       const QString &summary,
                       bool incrementUnread);
    /**
     * @brief 更新消息投递状态并持久化。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param state 新的投递状态。
     */
    void updateMessageState(const QString &peerId,
                            const QString &messageId,
                            Domain::DeliveryState state);
    /**
     * @brief 更新文件传输消息的进度、状态及可选路径并持久化。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的传输进度。
     * @param state 新的投递状态。
     * @param filePath 可用时提供本地文件路径。
     */
    void updateFileTransfer(const QString &peerId,
                            const QString &messageId,
                            qreal progress,
                            Domain::DeliveryState state,
                            const QString &filePath = {});

signals:
    /** @brief 节点集合或会话摘要发生变化时发出。 */
    void peersChanged();
    /**
     * @brief 发现此前未知的节点时发出。
     * @param peerId 新节点标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 已知节点的路由或在线状态发生变化时发出。
     * @param peerId 已更新节点的标识。
     */
    void peerUpdated(const QString &peerId);
    /**
     * @brief 新消息加入会话时发出。
     * @param message 已加入的消息。
     */
    void messageAdded(const Domain::Message &message);
    /**
     * @brief 消息投递状态发生变化时发出。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param state 新的投递状态。
     */
    void messageStateChanged(const QString &peerId,
                             const QString &messageId,
                             Domain::DeliveryState state);
    /**
     * @brief 文件传输消息发生变化时发出。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的传输进度。
     * @param state 新的投递状态。
     * @param filePath 可用时为本地文件路径。
     */
    void fileTransferChanged(const QString &peerId,
                             const QString &messageId,
                             qreal progress,
                             Domain::DeliveryState state,
                             const QString &filePath);
    /**
     * @brief 会话仓储操作失败时发出。
     * @param reason 便于用户阅读的失败原因。
     */
    void operationFailed(const QString &reason);

private:
    /**
     * @brief 将指定会话加载到内存缓存。
     * @param peerId 会话对应的节点标识。
     */
    void loadConversation(const QString &peerId);
    /**
     * @brief 更新内存中的会话摘要并持久化。
     * @param peerId 会话对应的节点标识。
     * @param lastMessage 最新消息预览。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读计数。
     */
    void updateConversation(const QString &peerId,
                            const QString &lastMessage,
                            const QDateTime &timestamp,
                            bool incrementUnread);
    /**
     * @brief 持久化领域消息。
     * @param message 待持久化的消息。
     */
    void persistMessage(const Domain::Message &message);
    /**
     * @brief 查找节点在列表中的索引。
     * @param peerId 待查找的节点标识。
     * @return 节点索引；节点未知时返回 @c -1。
     */
    [[nodiscard]] int peerIndex(const QString &peerId) const;
    /**
     * @brief 返回已知节点的可修改指针。
     * @param peerId 待查找的节点标识。
     * @return 指向节点的指针；节点未知时返回 @c nullptr。
     */
    [[nodiscard]] Domain::Peer *mutablePeer(const QString &peerId);
    /**
     * @brief 记录并广播数据仓储错误。
     * @param operation 失败的数据仓储操作名称。
     * @param error 仓储返回的错误说明，允许为空。
     */
    void reportRepositoryError(const char *operation, const QString &error);

    std::unique_ptr<IChatRepository> m_repository;
    QList<Domain::Peer> m_peers;
    QHash<QString, QList<Domain::Message>> m_conversations;
    QSet<QString> m_loadedConversations;
    bool m_repositoryReady = false;
};

#endif // CONVERSATIONSTORE_H
