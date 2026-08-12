/**
 * @file chattypes.h
 * @brief 定义联系人、统一会话、群组、消息及投递状态领域类型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef CHATTYPES_H
#define CHATTYPES_H

#include "networktypes.h"

#include <QDateTime>
#include <QList>
#include <QString>

#include <utility>

namespace Domain
{

/**
 * @brief 会话类型。
 */
enum class ConversationKind
{
    Direct, ///< 直接会话。
    Group   ///< 群组会话。
};

/**
 * @brief 消息类型。
 */
enum class MessageKind
{
    Text, ///< 文本消息。
    File  ///< 文件消息。
};

/**
 * @brief 投递状态。
 */
enum class DeliveryState
{
    Pending,      ///< 等待发送或重试。
    Sending,      ///< 发送中。
    Sent,         ///< 已发送。
    Partial,      ///< 部分送达。
    Received,     ///< 已接收。
    Transferring, ///< 文件发送中。
    Receiving,    ///< 文件接收中。
    Cancelled,    ///< 已取消。
    Failed        ///< 已失败。
};

/**
 * @brief 群成员角色。
 */
enum class GroupRole
{
    Owner, ///< 群主。
    Member ///< 普通群成员。
};

struct Peer
{
    Network::PeerEndpoint endpoint; ///< 联系人网络端点。
    bool online = false;            ///< 是否在线。
};

struct Conversation
{
    QString conversationId;                           ///< 会话标识。
    ConversationKind kind = ConversationKind::Direct; ///< 会话类型。
    QString peerId;                                   ///< 联系人标识；群聊时为空。
    QString title;                                    ///< 会话标题。
    QString lastMessage;                              ///< 最后一条消息摘要。
    QDateTime lastActivity;                           ///< 最近活动时间。
    int unreadCount = 0;                              ///< 未读消息数。
    int memberCount = 0;                              ///< 会话成员数。
};

struct GroupMember
{
    QString peerId;                     ///< 成员设备标识。
    QString displayName;                ///< 成员显示名称。
    GroupRole role = GroupRole::Member; ///< 成员角色。
};

struct Group
{
    QString groupId;            ///< 群组标识。
    QString name;               ///< 群组名称。
    QString ownerId;            ///< 群主设备标识。
    quint64 revision = 1;       ///< 群组版本号。
    QDateTime createdAt;        ///< 创建时间。
    QList<GroupMember> members; ///< 群成员列表。
};

struct Message
{
    QString messageId;                                     ///< 消息标识。
    QString conversationId;                                ///< 所属会话标识。
    QString senderId;                                      ///< 发送方设备标识。
    QString text;                                          ///< 文本内容。
    QDateTime timestamp;                                   ///< 消息时间。
    DeliveryState deliveryState = DeliveryState::Received; ///< 投递状态。
    MessageKind kind = MessageKind::Text;                  ///< 消息类型。
    QString fileName;                                      ///< 文件名。
    QString filePath;                                      ///< 本地文件路径。
    QString legacyFileSizeText;                            ///< 旧版文件大小文本。
    qint64 fileSize = 0;                                   ///< 文件大小（字节）。
    qreal fileProgress = 0.0;                              ///< 文件进度，范围为 0.0～1.0。
};

struct MessageDelivery
{
    QString messageId;                            ///< 消息标识。
    QString conversationId;                       ///< 会话标识。
    QString recipientId;                          ///< 接收方设备标识。
    DeliveryState state = DeliveryState::Pending; ///< 投递状态。
    QString errorMessage;                         ///< 错误信息。
    QDateTime lastAttempt;                        ///< 最近尝试时间。
};

struct OperationResult
{
    bool succeeded = true; ///< 是否成功。
    QString code;          ///< 错误码。
    QString message;       ///< 错误信息。
    QString value;         ///< 返回值。

    /**
     * @brief 判断操作是否成功完成。
     * @return 操作成功时返回 @c true，否则返回 @c false。
     */
    [[nodiscard]] explicit operator bool() const
    {
        return succeeded;
    }

    /**
     * @brief 创建失败的操作结果。
     * @param errorCode 稳定且可供程序识别的错误码。
     * @param errorMessage 便于用户阅读的错误说明。
     * @return 包含指定错误信息的失败结果。
     */
    [[nodiscard]] static OperationResult failure(QString errorCode, QString errorMessage)
    {
        OperationResult result;
        result.succeeded = false;
        result.code = std::move(errorCode);
        result.message = std::move(errorMessage);
        return result;
    }

    /**
     * @brief 创建成功的操作结果。
     * @param resultValue 操作产生的可选结果值。
     * @return 成功的操作结果。
     */
    [[nodiscard]] static OperationResult success(QString resultValue = {})
    {
        OperationResult result;
        result.value = std::move(resultValue);
        return result;
    }
};

/**
 * @brief 为指定联系人生成本地直接会话标识。
 * @param peerId 联系人设备标识。
 * @return 以 @c direct: 为前缀的稳定本地会话标识。
 */
[[nodiscard]] QString directConversationId(const QString &peerId);

/**
 * @brief 从直接会话标识提取联系人标识。
 * @param conversationId 待解析的会话标识。
 * @return 直接会话对应的联系人标识；格式无效时返回空字符串。
 */
[[nodiscard]] QString peerIdFromDirectConversation(const QString &conversationId);

/**
 * @brief 将会话类型转换为稳定名称。
 * @param kind 待转换的会话类型。
 * @return 用于持久化的类型名称。
 */
[[nodiscard]] QString conversationKindName(ConversationKind kind);

/**
 * @brief 从稳定名称解析会话类型。
 * @param name 持久化的类型名称。
 * @return 解析结果；未知名称按直接会话处理。
 */
[[nodiscard]] ConversationKind conversationKindFromName(const QString &name);

/**
 * @brief 将投递状态转换为用于持久化的名称。
 * @param state 待转换的投递状态。
 * @return @p state 对应的稳定文本表示。
 */
[[nodiscard]] QString deliveryStateName(DeliveryState state);

/**
 * @brief 将持久化的投递状态名称转换为枚举值。
 * @param name 投递状态的文本名称。
 * @return 解析后的投递状态；名称未知时返回失败状态。
 */
[[nodiscard]] DeliveryState deliveryStateFromName(const QString &name);

/**
 * @brief 将消息类型转换为用于持久化的名称。
 * @param kind 待转换的消息类型。
 * @return @p kind 对应的稳定文本表示。
 */
[[nodiscard]] QString messageKindName(MessageKind kind);

/**
 * @brief 将持久化的消息类型名称转换为枚举值。
 * @param name 消息类型的文本名称。
 * @return 解析后的消息类型；名称未知时返回文本类型。
 */
[[nodiscard]] MessageKind messageKindFromName(const QString &name);

/**
 * @brief 将群成员角色转换为用于持久化的名称。
 * @param role 待转换的成员角色。
 * @return @p role 对应的稳定文本表示。
 */
[[nodiscard]] QString groupRoleName(GroupRole role);

/**
 * @brief 将持久化的群成员角色转换为枚举值。
 * @param name 群成员角色名称。
 * @return 解析后的成员角色；名称未知时返回普通成员。
 */
[[nodiscard]] GroupRole groupRoleFromName(const QString &name);

} // namespace Domain

#endif // CHATTYPES_H
