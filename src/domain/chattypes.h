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

enum class ConversationKind
{
    Direct,
    Group
};

enum class MessageKind
{
    Text,
    File
};

enum class DeliveryState
{
    Pending,
    Sending,
    Sent,
    Partial,
    Received,
    Transferring,
    Receiving,
    Cancelled,
    Failed
};

enum class GroupRole
{
    Owner,
    Member
};

struct Peer
{
    Network::PeerEndpoint endpoint;
    bool online = false;
};

struct Conversation
{
    QString conversationId;
    ConversationKind kind = ConversationKind::Direct;
    QString peerId;
    QString title;
    QString lastMessage;
    QDateTime lastActivity;
    int unreadCount = 0;
    int memberCount = 0;
};

struct GroupMember
{
    QString peerId;
    QString displayName;
    GroupRole role = GroupRole::Member;
};

struct Group
{
    QString groupId;
    QString name;
    QString ownerId;
    quint64 revision = 1;
    QDateTime createdAt;
    QList<GroupMember> members;
};

struct Message
{
    QString messageId;
    QString conversationId;
    QString senderId;
    QString text;
    QDateTime timestamp;
    DeliveryState deliveryState = DeliveryState::Received;
    MessageKind kind = MessageKind::Text;
    QString fileName;
    QString filePath;
    QString legacyFileSizeText;
    qint64 fileSize = 0;
    qreal fileProgress = 0.0;
};

struct MessageDelivery
{
    QString messageId;
    QString conversationId;
    QString recipientId;
    DeliveryState state = DeliveryState::Pending;
    QString errorMessage;
    QDateTime lastAttempt;
};

struct OperationResult
{
    bool succeeded = true;
    QString code;
    QString message;
    QString value;

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
