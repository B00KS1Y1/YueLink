#include "chattypes.h"

namespace Domain
{

QString directConversationId(const QString &peerId)
{
    return peerId.isEmpty() ? QString{} : QStringLiteral("direct:%1").arg(peerId);
}

QString peerIdFromDirectConversation(const QString &conversationId)
{
    constexpr auto Prefix = "direct:";
    return conversationId.startsWith(QLatin1String(Prefix))
               ? conversationId.sliced(sizeof(Prefix) - 1)
               : QString{};
}

QString conversationKindName(ConversationKind kind)
{
    return kind == ConversationKind::Group ? QStringLiteral("group")
                                           : QStringLiteral("direct");
}

ConversationKind conversationKindFromName(const QString &name)
{
    return name == QLatin1String("group") ? ConversationKind::Group
                                          : ConversationKind::Direct;
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
        return DeliveryState::Pending;
    if (name == QLatin1String("sending"))
        return DeliveryState::Sending;
    if (name == QLatin1String("sent"))
        return DeliveryState::Sent;
    if (name == QLatin1String("partial"))
        return DeliveryState::Partial;
    if (name == QLatin1String("received"))
        return DeliveryState::Received;
    if (name == QLatin1String("transferring"))
        return DeliveryState::Transferring;
    if (name == QLatin1String("receiving"))
        return DeliveryState::Receiving;
    if (name == QLatin1String("cancelled"))
        return DeliveryState::Cancelled;
    return DeliveryState::Failed;
}

QString messageKindName(MessageKind kind)
{
    return kind == MessageKind::File ? QStringLiteral("file")
                                     : QStringLiteral("text");
}

MessageKind messageKindFromName(const QString &name)
{
    return name == QLatin1String("file") ? MessageKind::File
                                         : MessageKind::Text;
}

QString groupRoleName(GroupRole role)
{
    return role == GroupRole::Owner ? QStringLiteral("owner")
                                    : QStringLiteral("member");
}

GroupRole groupRoleFromName(const QString &name)
{
    return name == QLatin1String("owner") ? GroupRole::Owner
                                          : GroupRole::Member;
}

} // namespace Domain
