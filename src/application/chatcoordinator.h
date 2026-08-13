/**
 * @file chatcoordinator.h
 * @brief 声明统一会话、群聊、离线投递与文件传输应用协调器。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef CHATCOORDINATOR_H
#define CHATCOORDINATOR_H

#include "domain/chattypes.h"

#include <QHash>
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
     * @brief 构造协调器并接管发现、传输及仓储实现。
     * @param discovery 节点发现实现。
     * @param transport 聊天传输实现。
     * @param repository 数据仓储实现。
     * @param parent 可选的 QObject 父对象。
     */
    ChatCoordinator(std::unique_ptr<IPeerDiscovery> discovery,
                    std::unique_ptr<IChatTransport> transport,
                    std::unique_ptr<IChatRepository> repository,
                    QObject *parent = nullptr);
    /** @brief 停止服务并释放协调器。 */
    ~ChatCoordinator() override;

    /**
     * @brief 返回本地身份。
     * @return 本地身份副本。
     */
    [[nodiscard]] Network::LocalIdentity localIdentity() const;
    /**
     * @brief 返回本机头像绝对路径。
     * @return 有效头像路径；未设置时为空。
     */
    [[nodiscard]] QString localAvatarPath() const;
    /**
     * @brief 返回本机头像背景色。
     * @return 不透明十六进制颜色。
     */
    [[nodiscard]] QString localAvatarColor() const;
    /**
     * @brief 返回全部联系人。
     * @return 联系人列表。
     */
    [[nodiscard]] QList<Domain::Peer> peers() const;
    /**
     * @brief 返回全部会话。
     * @return 单聊与群聊会话列表。
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
     * @brief 返回群组成员。
     * @param groupId 群组标识。
     * @return 群成员列表。
     */
    [[nodiscard]] QList<Domain::GroupMember> groupMembers(const QString &groupId) const;
    /**
     * @brief 返回会话最近消息。
     * @param conversationId 会话标识。
     * @param limit 最大消息数量。
     * @return 消息列表。
     */
    [[nodiscard]] QList<Domain::Message> messages(const QString &conversationId, int limit = 500);
    /**
     * @brief 返回群消息逐成员发送统计。
     * @param messageId 消息标识。
     * @param[out] deliveredCount 接收已发送数量；允许为空。
     * @param[out] totalCount 接收总数；允许为空。
     */
    void deliveryCounts(const QString &messageId, int *deliveredCount, int *totalCount) const;
    /**
     * @brief 返回在线联系人数。
     * @return 在线联系人数。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回全部会话未读数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 返回网络服务运行状态。
     * @return 发现与传输均运行时返回 @c true。
     */
    [[nodiscard]] bool running() const;
    /**
     * @brief 返回最近应用错误。
     * @return 最近错误文本。
     */
    [[nodiscard]] QString lastError() const;

    /**
     * @brief 启动传输与发现服务。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult start();
    /** @brief 停止传输与发现服务。 */
    void stop();
    /**
     * @brief 立即广播一次节点探测。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult refreshPeerDiscovery();
    /**
     * @brief 标记会话已读。
     * @param conversationId 会话标识。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult markConversationRead(const QString &conversationId);
    /**
     * @brief 创建群聊并向在线成员发送快照。
     * @param name 群名称，1 到 64 个字符。
     * @param memberIds 至少两个、最多三十一个联系人标识。
     * @return 成功时 value 为新群会话标识。
     */
    [[nodiscard]] Domain::OperationResult createGroup(const QString &name, const QStringList &memberIds);
    /**
     * @brief 更新本地资料并保存。
     * @param displayName 显示名称。
     * @param avatarPath 本地头像绝对路径；空值清除头像。
     * @param avatarColor 头像背景色。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult updateLocalProfile(const QString &displayName, const QString &avatarPath, const QString &avatarColor);
    /**
     * @brief 向直接会话或群聊发送文本。
     * @param conversationId 会话标识。
     * @param text 消息文本。
     * @return 成功时 value 为消息标识。
     */
    [[nodiscard]] Domain::OperationResult sendText(const QString &conversationId, const QString &text);
    /**
     * @brief 向直接会话发送图片。
     * @param conversationId 直接会话标识。
     * @param filePath 本地图片路径。
     * @param caption 图片说明。
     * @return 成功时 value 为消息标识。
     */
    [[nodiscard]] Domain::OperationResult sendImage(const QString &conversationId,
                                                    const QString &filePath,
                                                    const QString &caption = {});
    /**
     * @brief 向直接会话发送多个图片。
     * @param conversationId 直接会话标识。
     * @param filePaths 本地图片路径列表。
     * @return 已接受的图片数量。
     */
    [[nodiscard]] int sendImages(const QString &conversationId,
                                 const QStringList &filePaths);
    /**
     * @brief 向会话发送表情。
     * @param conversationId 会话标识。
     * @param packageId 表情包标识。
     * @param emojiId 表情标识。
     * @param fallbackText 回退文本。
     * @return 成功时 value 为消息标识。
     */
    [[nodiscard]] Domain::OperationResult sendEmoji(const QString &conversationId,
                                                    const QString &packageId,
                                                    const QString &emojiId,
                                                    const QString &fallbackText);
    /**
     * @brief 向直接会话发送文件。
     * @param conversationId 直接会话标识。
     * @param filePath 本地文件路径。
     * @return 结构化操作结果；群聊会返回不支持错误。
     */
    [[nodiscard]] Domain::OperationResult sendFile(const QString &conversationId, const QString &filePath);
    /**
     * @brief 向直接会话发送多个文件。
     * @param conversationId 直接会话标识。
     * @param filePaths 本地文件路径列表。
     * @return 已接受的文件数量。
     */
    [[nodiscard]] int sendFiles(const QString &conversationId, const QStringList &filePaths);
    /**
     * @brief 取消直接会话中的文件传输。
     * @param conversationId 直接会话标识。
     * @param transferId 文件传输标识。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult cancelFileTransfer(const QString &conversationId, const QString &transferId);
    /**
     * @brief 接受直接会话中等待确认的文件传输请求。
     * @param conversationId 直接会话标识。
     * @param transferId 文件传输标识。
     * @return 结构化操作结果。
     */
    [[nodiscard]] Domain::OperationResult acceptFileTransfer(
        const QString &conversationId,
        const QString &transferId);

signals:
    /** @brief 本地资料发生变化时发出。 */
    void localIdentityChanged();
    /** @brief 联系人集合或状态发生变化时发出。 */
    void peersChanged();
    /** @brief 会话集合或摘要发生变化时发出。 */
    void conversationsChanged();
    /** @brief 网络运行状态发生变化时发出。 */
    void runningChanged();
    /** @brief 最近错误发生变化时发出。 */
    void lastErrorChanged();
    /**
     * @brief 发现联系人时发出。
     * @param peerId 联系人设备标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 联系人更新时发出。
     * @param peerId 联系人设备标识。
     */
    void peerUpdated(const QString &peerId);
    /**
     * @brief 群组更新时发出。
     * @param groupId 群组标识。
     */
    void groupChanged(const QString &groupId);
    /**
     * @brief 新消息加入时发出。
     * @param message 新消息。
     */
    void messageAdded(const Domain::Message &message);
    /**
     * @brief 消息状态更新时发出。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param state 新状态。
     */
    void messageStateChanged(const QString &conversationId, const QString &messageId, Domain::DeliveryState state);
    /**
     * @brief 逐成员投递统计变化时发出。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param deliveredCount 已发送数量。
     * @param totalCount 接收方总数。
     */
    void deliveryChanged(const QString &conversationId, const QString &messageId, int deliveredCount, int totalCount);
    /**
     * @brief 文件传输消息变化时发出。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param progress 进度。
     * @param state 新状态。
     * @param filePath 本地路径。
     */
    void fileTransferChanged(const QString &conversationId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath);
    /**
     * @brief 收到消息时发出。
     * @param conversationId 会话标识。
     * @param text 消息文本。
     */
    void messageReceived(const QString &conversationId, const QString &text);
    /**
     * @brief 消息发送失败或仍待发送时发出。
     * @param conversationId 会话标识。
     * @param reason 原因。
     */
    void sendFailed(const QString &conversationId, const QString &reason);
    /**
     * @brief 接收文件成功时发出。
     * @param conversationId 直接会话标识。
     * @param filePath 本地路径。
     */
    void fileReceived(const QString &conversationId, const QString &filePath);
    /**
     * @brief 文件传输失败时发出。
     * @param conversationId 直接会话标识。
     * @param reason 原因。
     * @param incoming 是否为接收方向。
     */
    void fileTransferFailed(const QString &conversationId, const QString &reason, bool incoming);
    /**
     * @brief 一般性操作失败时发出。
     * @param reason 原因。
     */
    void operationFailed(const QString &reason);

private:
    /** @brief 连接发现、传输、仓储和文件协调器事件。 */
    void connectServices();
    /**
     * @brief 加载或创建本地身份。
     * @return 身份可用时返回 @c true。
     */
    [[nodiscard]] bool initializeIdentity();
    /**
     * @brief 处理收到的消息。
     * @param message 领域消息。
     * @param sender 发送方端点。
     */
    void handleMessage(const Domain::Message &message,
                       const Network::PeerEndpoint &sender);
    /**
     * @brief 发送已构造的消息。
     * @param conversationId 目标会话标识。
     * @param payload 类型化载荷。
     * @param localPath 附件本地路径；非附件消息为空。
     * @return 成功时 value 为消息标识。
     */
    [[nodiscard]] Domain::OperationResult sendPayload(
        const QString &conversationId,
        Domain::MessagePayload payload,
        const QString &localPath = {});
    /**
     * @brief 构造附件描述。
     * @param filePath 本地文件路径。
     * @param[out] descriptor 接收附件描述。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 文件有效且可读取时返回 @c true。
     */
    [[nodiscard]] bool attachmentDescriptor(const QString &filePath,
                                            Domain::AttachmentDescriptor *descriptor,
                                            QString *errorMessage) const;
    /**
     * @brief 处理收到的群组快照。
     * @param snapshot 网络群组快照。
     */
    void handleGroupSnapshot(const Network::GroupSnapshot &snapshot);
    /**
     * @brief 向指定联系人发送其参与且本机拥有的群组快照。
     * @param peer 联系人。
     */
    void synchronizeOwnedGroupsWithPeer(const Domain::Peer &peer);
    /**
     * @brief 尝试发送指定联系人的所有待投递群消息。
     * @param peerId 联系人设备标识。
     */
    void dispatchPendingToPeer(const QString &peerId);
    /**
     * @brief 将领域群组转换为网络快照。
     * @param group 领域群组。
     * @return 可发送的网络快照。
     */
    [[nodiscard]] Network::GroupSnapshot networkSnapshot(const Domain::Group &group) const;
    /**
     * @brief 更新公开的最近错误。
     * @param error 新错误；空值清除。
     */
    void setLastError(const QString &error);
    /**
     * @brief 解析直接会话对应的在线联系人。
     * @param conversationId 直接会话标识。
     * @param[out] peerRecord 成功时接收联系人。
     * @param fileOperation 是否为文件操作，用于选择错误信号。
     * @return 联系人在线且服务运行时返回 @c true。
     */
    [[nodiscard]] bool resolveOnlineDirectPeer(const QString &conversationId, Domain::Peer *peerRecord, bool fileOperation);

    std::unique_ptr<IPeerDiscovery> m_discovery;
    std::unique_ptr<IChatTransport> m_transport;
    std::unique_ptr<ConversationStore> m_conversations;
    std::unique_ptr<TransferCoordinator> m_transfers;
    Network::LocalIdentity m_identity;
    QString m_localAvatarPath;
    QString m_localAvatarColor;
    QString m_lastError;
    QMultiHash<QString, Domain::Message> m_pendingGroupMessages;
    bool m_identityReady = false;
    bool m_running = false;
};

#endif // CHATCOORDINATOR_H
