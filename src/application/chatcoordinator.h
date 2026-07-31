/**
 * @file chatcoordinator.h
 * @brief 声明聊天应用生命周期与文本消息编排协调器。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef CHATCOORDINATOR_H
#define CHATCOORDINATOR_H

#include "core/chattypes.h"

#include <QList>
#include <QObject>
#include <QStringList>

#include <memory>

class ConversationStore;
class IChatRepository;
class IChatTransport;
class IPeerDiscovery;
class TransferCoordinator;

class ChatCoordinator final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造聊天应用协调器并接管基础设施适配器所有权。
     * @param discovery 节点发现实现。
     * @param transport 聊天传输实现。
     * @param repository 会话数据仓储实现。
     * @param parent 可选的 QObject 父对象。
     */
    ChatCoordinator(std::unique_ptr<IPeerDiscovery> discovery,
                    std::unique_ptr<IChatTransport> transport,
                    std::unique_ptr<IChatRepository> repository,
                    QObject *parent = nullptr);
    /** @brief 停止聊天服务并释放其拥有的协作对象。 */
    ~ChatCoordinator() override;

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
     * @param[out] result 找到时接收匹配节点；允许传入 @c nullptr。
     * @return 节点存在时返回 @c true。
     */
    [[nodiscard]] bool peer(const QString &peerId, Domain::Peer *result) const;
    /**
     * @brief 返回指定会话的最近消息。
     * @param peerId 会话对应的节点标识。
     * @param limit 最多返回的消息数量。
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
     * @brief 返回所有会话未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 返回发现与传输服务是否正在运行。
     * @return 两项服务均正常运行时返回 @c true。
     */
    [[nodiscard]] bool running() const;
    /**
     * @brief 返回最近一次应用协调错误。
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
     * @brief 更新本地显示名称并保存到 identity.json。
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
    [[nodiscard]] Domain::OperationResult sendText(const QString &peerId,
                                                   const QString &text);
    /**
     * @brief 开始向指定节点发送本地文件。
     * @param peerId 目标节点标识。
     * @param filePath 本地文件路径。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult sendFile(const QString &peerId,
                                                   const QString &filePath);
    /**
     * @brief 开始向指定节点发送多个本地文件。
     * @param peerId 目标节点标识。
     * @param filePaths 本地文件路径列表。
     * @return 已接受发送的文件数量。
     */
    [[nodiscard]] int sendFiles(const QString &peerId,
                                const QStringList &filePaths);
    /**
     * @brief 取消正在进行的文件传输。
     * @param peerId 远端节点标识。
     * @param transferId 文件传输标识。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult cancelFileTransfer(const QString &peerId,
                                                             const QString &transferId);

signals:
    /** @brief 本地身份发生变化后发出。 */
    void localIdentityChanged();
    /** @brief 节点集合或会话摘要发生变化时发出。 */
    void peersChanged();
    /** @brief 运行状态发生变化时发出。 */
    void runningChanged();
    /** @brief 最近错误发生变化时发出。 */
    void lastErrorChanged();
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
     * @param progress 传输进度。
     * @param state 新的投递状态。
     * @param filePath 可用时为本地文件路径。
     */
    void fileTransferChanged(const QString &peerId,
                             const QString &messageId,
                             qreal progress,
                             Domain::DeliveryState state,
                             const QString &filePath);
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
    void fileTransferFailed(const QString &peerId,
                            const QString &reason,
                            bool incoming);
    /**
     * @brief 一般性应用操作失败时发出。
     * @param reason 便于用户阅读的失败原因。
     */
    void operationFailed(const QString &reason);

private:
    /** @brief 连接发现、传输、会话存储与文件协调器事件。 */
    void connectServices();
    /**
     * @brief 从 identity.json 加载或创建本地身份。
     * @return 身份可用且所需持久化成功时返回 @c true。
     */
    [[nodiscard]] bool initializeIdentity();
    /**
     * @brief 处理传输层收到的文本消息。
     * @param message 收到的消息事件。
     */
    void handleTextMessage(const Network::TextMessage &message);
    /**
     * @brief 更新公开的最近错误。
     * @param error 新错误文本；空字符串表示清除错误。
     */
    void setLastError(const QString &error);
    /**
     * @brief 检查目标节点当前是否允许发送数据。
     * @param peerId 目标节点标识。
     * @param[out] peerRecord 成功时接收节点信息。
     * @param fileOperation 是否为文件操作，用于选择错误信号。
     * @return 节点存在、在线且服务运行时返回 @c true。
     */
    [[nodiscard]] bool resolveOnlinePeer(const QString &peerId,
                                         Domain::Peer *peerRecord,
                                         bool fileOperation);

    std::unique_ptr<IPeerDiscovery> m_discovery;
    std::unique_ptr<IChatTransport> m_transport;
    std::unique_ptr<ConversationStore> m_conversations;
    std::unique_ptr<TransferCoordinator> m_transfers;
    Network::LocalIdentity m_identity;
    QString m_lastError;
    bool m_identityReady = false;
    bool m_running = false;
};

#endif // CHATCOORDINATOR_H
