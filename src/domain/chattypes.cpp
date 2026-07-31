#include "chattypes.h"

namespace Domain
{

QString deliveryStateName(DeliveryState state)
{
    switch (state)
    {
    case DeliveryState::Sending:
        return QStringLiteral("sending");
    case DeliveryState::Sent:
        return QStringLiteral("sent");
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
    if (name == QLatin1String("sending"))
    {
        return DeliveryState::Sending;
    }
    if (name == QLatin1String("sent"))
    {
        return DeliveryState::Sent;
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
    if (name == QLatin1String("failed"))
    {
        return DeliveryState::Failed;
    }
    return DeliveryState::Received;
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

} // namespace Domain
