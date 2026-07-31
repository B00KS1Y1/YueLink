/**
 * @file chattypes.h
 * @brief 定义聊天服务与前端共用的领域类型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef CHATTYPES_H
#define CHATTYPES_H

#include "networktypes.h"

#include <QDateTime>
#include <QString>

#include <utility>

namespace Domain
{

enum class MessageKind
{
    Text,
    File
};

enum class DeliveryState
{
    Sending,
    Sent,
    Received,
    Transferring,
    Receiving,
    Cancelled,
    Failed
};

struct Peer
{
    Network::PeerEndpoint endpoint;
    QString lastMessage;
    QDateTime lastActivity;
    int unreadCount = 0;
    bool online = false;
};

struct Message
{
    QString messageId;
    QString peerId;
    QString text;
    QDateTime timestamp;
    DeliveryState deliveryState = DeliveryState::Received;
    MessageKind kind = MessageKind::Text;
    QString fileName;
    QString filePath;
    QString legacyFileSizeText;
    qint64 fileSize = 0;
    qreal fileProgress = 0.0;
    bool fromMe = false;
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
 * @brief 将投递状态转换为用于持久化的名称。
 * @param state 待转换的投递状态。
 * @return @p state 对应的稳定文本表示。
 */
[[nodiscard]] QString deliveryStateName(DeliveryState state);

/**
 * @brief 将持久化的投递状态名称转换为枚举值。
 * @param name 投递状态的文本名称。
 * @return 解析后的投递状态；名称未知时返回默认状态。
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
 * @return 解析后的消息类型；名称未知时返回默认类型。
 */
[[nodiscard]] MessageKind messageKindFromName(const QString &name);

} // namespace Domain

#endif // CHATTYPES_H
