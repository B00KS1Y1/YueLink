#include "chattypes.h"

#include <QJsonValue>

#include <type_traits>

namespace Domain
{

QString directConversationId(const QString &peerId)
{
    return peerId.isEmpty() ? QString{} : QStringLiteral("direct:%1").arg(peerId);
}

QString peerIdFromDirectConversation(const QString &conversationId)
{
    constexpr auto Prefix = "direct:";
    return conversationId.startsWith(QLatin1String(Prefix)) ? conversationId.sliced(sizeof(Prefix) - 1) : QString{};
}

QString conversationKindName(ConversationKind kind)
{
    return kind == ConversationKind::Group ? QStringLiteral("group") : QStringLiteral("direct");
}

ConversationKind conversationKindFromName(const QString &name)
{
    return name == QLatin1String("group") ? ConversationKind::Group : ConversationKind::Direct;
}

QString deliveryStateName(DeliveryState state)
{
    switch (state)
    {
    case DeliveryState::Pending:
        return QStringLiteral("pending");
    case DeliveryState::Sending:
        return QStringLiteral("sending");
    case DeliveryState::Sent:
        return QStringLiteral("sent");
    case DeliveryState::Partial:
        return QStringLiteral("partial");
    case DeliveryState::Received:
        return QStringLiteral("received");
    case DeliveryState::Transferring:
        return QStringLiteral("transferring");
    case DeliveryState::Receiving:
        return QStringLiteral("receiving");
    case DeliveryState::Cancelled:
        return QStringLiteral("cancelled");
    case DeliveryState::Failed:
        return QStringLiteral("failed");
    }
    return QStringLiteral("failed");
}

DeliveryState deliveryStateFromName(const QString &name)
{
    if (name == QLatin1String("pending"))
    {
        return DeliveryState::Pending;
    }
    if (name == QLatin1String("sending"))
    {
        return DeliveryState::Sending;
    }
    if (name == QLatin1String("sent"))
    {
        return DeliveryState::Sent;
    }
    if (name == QLatin1String("partial"))
    {
        return DeliveryState::Partial;
    }
    if (name == QLatin1String("received"))
    {
        return DeliveryState::Received;
    }
    if (name == QLatin1String("transferring"))
    {
        return DeliveryState::Transferring;
    }
    if (name == QLatin1String("receiving"))
    {
        return DeliveryState::Receiving;
    }
    if (name == QLatin1String("cancelled"))
    {
        return DeliveryState::Cancelled;
    }
    return DeliveryState::Failed;
}

QString messageKindName(MessageKind kind)
{
    switch (kind)
    {
    case MessageKind::Text:
        return QStringLiteral("text");
    case MessageKind::Image:
        return QStringLiteral("image");
    case MessageKind::File:
        return QStringLiteral("file");
    case MessageKind::Emoji:
        return QStringLiteral("emoji");
    }
    return QStringLiteral("text");
}

MessageKind messageKindFromName(const QString &name)
{
    if (name == QLatin1String("image"))
    {
        return MessageKind::Image;
    }
    if (name == QLatin1String("file"))
    {
        return MessageKind::File;
    }
    if (name == QLatin1String("emoji"))
    {
        return MessageKind::Emoji;
    }
    return MessageKind::Text;
}

MessageKind messageKind(const Message &message)
{
    return std::visit([](const auto &payload) {
        using Payload = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<Payload, TextPayload>)
        {
            return MessageKind::Text;
        }
        else if constexpr (std::is_same_v<Payload, ImagePayload>)
        {
            return MessageKind::Image;
        }
        else if constexpr (std::is_same_v<Payload, FilePayload>)
        {
            return MessageKind::File;
        }
        else
        {
            return MessageKind::Emoji;
        }
    }, message.payload);
}

QString messageText(const Message &message)
{
    return std::visit([](const auto &payload) -> QString {
        using Payload = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<Payload, TextPayload>)
        {
            return payload.text;
        }
        else if constexpr (std::is_same_v<Payload, ImagePayload>)
        {
            return payload.caption;
        }
        else if constexpr (std::is_same_v<Payload, EmojiPayload>)
        {
            return payload.fallbackText;
        }
        else
        {
            return {};
        }
    }, message.payload);
}

const AttachmentDescriptor *messageAttachment(const Message &message)
{
    return std::visit([](const auto &payload) -> const AttachmentDescriptor * {
        using Payload = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<Payload, ImagePayload> || std::is_same_v<Payload, FilePayload>)
        {
            return &payload.attachment;
        }
        else
        {
            return nullptr;
        }
    }, message.payload);
}

QString messageSummary(const Message &message)
{
    return std::visit([](const auto &payload) -> QString {
        using Payload = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<Payload, TextPayload>)
        {
            return payload.text;
        }
        else if constexpr (std::is_same_v<Payload, ImagePayload>)
        {
            return payload.caption.isEmpty() ? QStringLiteral("[图片] %1").arg(payload.attachment.fileName)
                                             : payload.caption;
        }
        else if constexpr (std::is_same_v<Payload, FilePayload>)
        {
            return QStringLiteral("[文件] %1").arg(payload.attachment.fileName);
        }
        else
        {
            return payload.fallbackText;
        }
    }, message.payload);
}

QJsonObject messagePayloadToJson(const MessagePayload &payload)
{
    return std::visit([](const auto &value) {
        using Payload = std::decay_t<decltype(value)>;
        QJsonObject object;
        if constexpr (std::is_same_v<Payload, TextPayload>)
        {
            object.insert(QStringLiteral("text"), value.text);
        }
        else if constexpr (std::is_same_v<Payload, ImagePayload> || std::is_same_v<Payload, FilePayload>)
        {
            object.insert(QStringLiteral("attachmentId"), value.attachment.attachmentId);
            object.insert(QStringLiteral("fileName"), value.attachment.fileName);
            object.insert(QStringLiteral("mimeType"), value.attachment.mimeType);
            object.insert(QStringLiteral("fileSize"), value.attachment.fileSize);
            object.insert(QStringLiteral("sha256"), QString::fromLatin1(value.attachment.sha256.toHex()));
            if constexpr (std::is_same_v<Payload, ImagePayload>)
            {
                object.insert(QStringLiteral("width"), value.dimensions.width());
                object.insert(QStringLiteral("height"), value.dimensions.height());
                object.insert(QStringLiteral("caption"), value.caption);
            }
        }
        else
        {
            object.insert(QStringLiteral("packageId"), value.packageId);
            object.insert(QStringLiteral("emojiId"), value.emojiId);
            object.insert(QStringLiteral("fallbackText"), value.fallbackText);
        }
        return object;
    }, payload);
}

std::optional<MessagePayload> messagePayloadFromJson(MessageKind kind,
                                                     const QJsonObject &object)
{
    switch (kind)
    {
    case MessageKind::Text:
    {
        const QString text = object.value(QStringLiteral("text")).toString();
        return text.isEmpty() || text.size() > 2000 ? std::nullopt
                              : std::optional<MessagePayload>{TextPayload{text}};
    }
    case MessageKind::Image:
    case MessageKind::File:
    {
        AttachmentDescriptor attachment;
        attachment.attachmentId = object.value(QStringLiteral("attachmentId")).toString();
        attachment.fileName = object.value(QStringLiteral("fileName")).toString();
        attachment.mimeType = object.value(QStringLiteral("mimeType")).toString();
        attachment.fileSize = object.value(QStringLiteral("fileSize")).toInteger(-1);
        attachment.sha256 = QByteArray::fromHex(object.value(QStringLiteral("sha256")).toString().toLatin1());
        if (attachment.attachmentId.isEmpty() || attachment.fileName.isEmpty()
            || attachment.attachmentId.size() > 128
            || attachment.fileName.size() > 255
            || attachment.mimeType.isEmpty() || attachment.mimeType.size() > 128
            || attachment.fileSize < 0
            || attachment.sha256.size() != 32)
        {
            return std::nullopt;
        }
        if (kind == MessageKind::Image)
        {
            const QSize dimensions(object.value(QStringLiteral("width")).toInt(),
                                   object.value(QStringLiteral("height")).toInt());
            const QString caption = object.value(QStringLiteral("caption")).toString();
            if (!dimensions.isValid() || dimensions.width() > 100000
                || dimensions.height() > 100000 || caption.size() > 2000)
            {
                return std::nullopt;
            }
            return ImagePayload{std::move(attachment), dimensions, caption};
        }
        return FilePayload{std::move(attachment)};
    }
    case MessageKind::Emoji:
    {
        EmojiPayload emoji;
        emoji.packageId = object.value(QStringLiteral("packageId")).toString();
        emoji.emojiId = object.value(QStringLiteral("emojiId")).toString();
        emoji.fallbackText = object.value(QStringLiteral("fallbackText")).toString();
        if (emoji.packageId.isEmpty() || emoji.packageId.size() > 128
            || emoji.emojiId.isEmpty() || emoji.emojiId.size() > 128
            || emoji.fallbackText.isEmpty() || emoji.fallbackText.size() > 64)
        {
            return std::nullopt;
        }
        return emoji;
    }
    }
    return std::nullopt;
}

QString groupRoleName(GroupRole role)
{
    return role == GroupRole::Owner ? QStringLiteral("owner") : QStringLiteral("member");
}

GroupRole groupRoleFromName(const QString &name)
{
    return name == QLatin1String("owner") ? GroupRole::Owner : GroupRole::Member;
}

} // namespace Domain
