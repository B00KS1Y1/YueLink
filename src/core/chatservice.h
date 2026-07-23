/**
 * @file chatservice.h
 * @brief 声明 GUI 与 CLI 共用的聊天应用服务。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "chattypes.h"

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
    /**
     * @brief 构造共享聊天应用服务。
     * @param discovery 由服务接管所有权的节点发现实现。
     * @param transport 由服务接管所有权的聊天传输实现。
     * @param repository 由服务接管所有权的聊天数据仓储实现。
     * @param identityStore 由服务接管所有权的身份存储实现。
     * @param parent 可选的 QObject 父对象。
     */
    ChatService(std::unique_ptr<IPeerDiscovery> discovery,
                std::unique_ptr<IChatTransport> transport,
                std::unique_ptr<IChatRepository> repository,
                std::unique_ptr<IIdentityStore> identityStore,
                QObject *parent = nullptr);
    /** @brief 停止服务并释放其拥有的协作对象。 */
    ~ChatService() override;

    /**
     * @brief 返回当前本地身份的副本。
     * @return 当前本地身份。
     */
    [[nodiscard]] Network::LocalIdentity localIdentity() const;
    /**
     * @brief 返回全部已知节点。
     * @return 已知节点列表。
     */
    [[nodiscard]] QList<Domain::Peer> peers() const;
    /**
     * @brief 根据标识查找节点。
     * @param peerId 待查找的节点标识。
     * @param[out] result 找到时接收匹配的节点信息。
     * @return 节点存在时返回 @c true。
     */
    [[nodiscard]] bool peer(const QString &peerId, Domain::Peer *result) const;
    /**
     * @brief 返回指定会话的最近消息。
     * @param peerId 会话对应的节点标识。
     * @param limit 最多返回的消息数量。
     * @return 按数据仓储与服务约定排序的消息列表。
     */
    [[nodiscard]] QList<Domain::Message> messages(const QString &peerId, int limit = 500);
    /**
     * @brief 返回当前在线的节点数量。
     * @return 在线节点数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有节点的未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 返回节点发现与传输服务是否正在运行。
     * @return 两项服务均正常运行时返回 @c true。
     */
    [[nodiscard]] bool running() const;
    /**
     * @brief 返回最近一次应用服务错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] QString lastError() const;

    /**
     * @brief 启动传输服务与节点发现。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult start();
    /** @brief 停止传输服务与节点发现。 */
    void stop();
    /**
     * @brief 将指定会话标记为已读。
     * @param peerId 会话对应的节点标识。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult markConversationRead(const QString &peerId);
    /**
     * @brief 更新并持久化本地显示名称。
     * @param displayName 新的显示名称。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult updateLocalProfile(const QString &displayName);
    /**
     * @brief 向指定节点发送文本消息。
     * @param peerId 目标节点标识。
     * @param text 消息内容。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult sendText(const QString &peerId, const QString &text);
    /**
     * @brief 开始向指定节点发送本地文件。
     * @param peerId 目标节点标识。
     * @param filePath 本地文件路径。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult sendFile(const QString &peerId, const QString &filePath);
    /**
     * @brief 开始向指定节点发送多个本地文件。
     * @param peerId 目标节点标识。
     * @param filePaths 本地文件路径列表。
     * @return 已接受发送的文件传输数量。
     */
    [[nodiscard]] int sendFiles(const QString &peerId, const QStringList &filePaths);
    /**
     * @brief 取消正在进行的文件传输。
     * @param peerId 远端节点标识。
     * @param transferId 文件传输标识。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult cancelFileTransfer(const QString &peerId, const QString &transferId);

signals:
    /** @brief 本地身份发生变化后发出。 */
    void localIdentityChanged();
    /** @brief 节点集合发生变化时发出。 */
    void peersChanged();
    /**
     * @brief 会话发生变化时发出。
     * @param peerId 受影响的节点标识。
     */
    void conversationChanged(const QString &peerId);
    /** @brief 运行状态发生变化时发出。 */
    void runningChanged();
    /** @brief 最近错误发生变化时发出。 */
    void lastErrorChanged();
    /**
     * @brief 发现此前未知的节点时发出。
     * @param peerId 已发现节点的标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 已知节点的信息发生变化时发出。
     * @param peerId 已更新节点的标识。
     */
    void peerUpdated(const QString &peerId);
    /**
     * @brief 收到文本消息时发出。
     * @param peerId 发送方节点标识。
     * @param text 收到的消息文本。
     */
    void messageReceived(const QString &peerId, const QString &text);
    /**
     * @brief 文本消息发送失败时发出。
     * @param peerId 目标节点标识。
     * @param reason 便于用户阅读的失败原因。
     */
    void sendFailed(const QString &peerId, const QString &reason);
    /**
     * @brief 接收文件成功保存后发出。
     * @param peerId 发送方节点标识。
     * @param filePath 接收文件的本地路径。
     */
    void fileReceived(const QString &peerId, const QString &filePath);
    /**
     * @brief 文件传输失败时发出。
     * @param peerId 远端节点标识。
     * @param reason 便于用户阅读的失败原因。
     * @param incoming 失败的传输是否为接收方向。
     */
    void fileTransferFailed(const QString &peerId, const QString &reason, bool incoming);
    /**
     * @brief 一般性服务操作失败时发出。
     * @param reason 便于用户阅读的失败原因。
     */
    void operationFailed(const QString &reason);

private:
    /** @brief 连接节点发现、传输与数据仓储的事件处理器。 */
    void connectServices();
    /** @brief 加载或创建本地身份。 */
    void initializeIdentity();
    /** @brief 初始化数据仓储并恢复已持久化的节点。 */
    void initializeRepository();
    /**
     * @brief 将指定会话加载到内存缓存。
     * @param peerId 会话对应的节点标识。
     */
    void loadConversation(const QString &peerId);
    /**
     * @brief 持久化已发现的节点。
     * @param peer 待持久化的节点信息。
     */
    void persistPeer(const Network::PeerEndpoint &peer);
    /**
     * @brief 持久化会话摘要。
     * @param peerId 会话对应的节点标识。
     * @param lastMessage 最新消息预览。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读计数。
     */
    void persistConversation(const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread);
    /**
     * @brief 持久化领域消息。
     * @param message 待持久化的消息。
     */
    void persistMessage(const Domain::Message &message);
    /**
     * @brief 持久化消息投递状态变化。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param state 新的投递状态。
     */
    void persistDeliveryStatus(const QString &peerId, const QString &messageId, Domain::DeliveryState state);
    /**
     * @brief 持久化文件传输进度与状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的传输进度。
     * @param state 新的投递状态。
     * @param filePath 可用时提供本地文件路径。
     */
    void persistFileTransfer(const QString &peerId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath = {});
    /**
     * @brief 记录数据仓储操作失败信息。
     * @param operation 失败的数据仓储操作名称。
     * @param error 便于用户阅读的数据仓储错误。
     */
    void logRepositoryError(const char *operation, const QString &error) const;
    /**
     * @brief 更新对外公开的最近错误。
     * @param error 新的错误文本；传入空字符串表示清除错误。
     */
    void setLastError(const QString &error);
    /**
     * @brief 新增或刷新已观察到的节点。
     * @param peer 已观察到的节点信息。
     */
    void observePeer(const Network::PeerEndpoint &peer);
    /**
     * @brief 将已知节点标记为离线。
     * @param peerId 已不可用节点的标识。
     */
    void markPeerOffline(const QString &peerId);
    /**
     * @brief 处理传输层收到的文本消息。
     * @param message 收到的消息事件。
     */
    void handleTextMessage(const Network::TextMessage &message);
    /**
     * @brief 处理文件传输开始事件。
     * @param transfer 文件传输元数据。
     */
    void handleFileTransferStarted(const Network::FileTransferInfo &transfer);
    /**
     * @brief 处理文件传输进度更新事件。
     * @param progress 文件传输进度事件。
     */
    void handleFileTransferProgress(const Network::FileTransferProgress &progress);
    /**
     * @brief 处理文件传输完成、失败或取消事件。
     * @param result 最终传输结果。
     */
    void handleFileTransferResult(const Network::FileTransferResult &result);
    /**
     * @brief 更新内存中的会话摘要。
     * @param peerId 会话对应的节点标识。
     * @param lastMessage 最新消息预览。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读计数。
     */
    void updateConversation(const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread);
    /**
     * @brief 更新内存中的消息投递状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param state 新的投递状态。
     */
    void updateMessageState(const QString &peerId, const QString &messageId, Domain::DeliveryState state);
    /**
     * @brief 更新内存中的文件传输消息状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的传输进度。
     * @param state 新的投递状态。
     * @param filePath 可用时提供本地文件路径。
     */
    void updateFileTransfer(const QString &peerId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath = {});
    /**
     * @brief 查找节点在列表中的索引。
     * @param peerId 待查找的节点标识。
     * @return 节点索引；节点未知时返回 @c -1。
     */
    [[nodiscard]] int peerIndex(const QString &peerId) const;
    /**
     * @brief 返回已知节点的可修改访问指针。
     * @param peerId 待查找的节点标识。
     * @return 指向节点的指针；节点未知时返回 @c nullptr。
     */
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
