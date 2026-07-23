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

    [[nodiscard]] explicit operator bool() const
    {
        return succeeded;
    }

    [[nodiscard]] static OperationResult failure(QString errorCode, QString errorMessage)
    {
        OperationResult result;
        result.succeeded = false;
        result.code = std::move(errorCode);
        result.message = std::move(errorMessage);
        return result;
    }

    [[nodiscard]] static OperationResult success(QString resultValue = {})
    {
        OperationResult result;
        result.value = std::move(resultValue);
        return result;
    }
};

[[nodiscard]] QString deliveryStateName(DeliveryState state);
[[nodiscard]] DeliveryState deliveryStateFromName(const QString &name);
[[nodiscard]] QString messageKindName(MessageKind kind);
[[nodiscard]] MessageKind messageKindFromName(const QString &name);

} // namespace Domain

#endif // CHATTYPES_H
