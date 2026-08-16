#include "wireprotocol.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QtEndian>

namespace
{
constexpr auto ProtocolName = "YueLink";

QByteArray frame(const QJsonObject &object)
{
    const QByteArray payload = QJsonDocument(object).toJson(QJsonDocument::Compact);
    QByteArray framedPayload(4, '\0');
    qToBigEndian<quint32>(static_cast<quint32>(payload.size()), reinterpret_cast<uchar *>(framedPayload.data()));
    framedPayload.append(payload);
    return framedPayload;
}

QJsonObject
envelope(const Network::LocalIdentity &identity, quint16 senderPort, const Network::PeerEndpoint &recipient, const QString &type, const QDateTime &timestamp)
{
    return {{QStringLiteral("app"), QString::fromLatin1(ProtocolName)},
            {QStringLiteral("version"), Network::WireProtocol::ProtocolVersion},
            {QStringLiteral("type"), type},
            {QStringLiteral("senderId"), identity.deviceId},
            {QStringLiteral("senderName"), identity.displayName},
            {QStringLiteral("senderPort"), senderPort},
            {QStringLiteral("recipientId"), recipient.peerId},
            {QStringLiteral("timestamp"), timestamp.toUTC().toString(Qt::ISODateWithMs)}};
}
} // namespace

QByteArray Network::WireProtocol::messageFrame(const LocalIdentity &identity, quint16 senderPort, const PeerEndpoint &recipient, const Domain::Message &message)
{
    QJsonObject object = envelope(identity, senderPort, recipient, QStringLiteral("message"), message.metadata.timestamp);
    object.insert(QStringLiteral("messageId"), message.metadata.messageId);
    object.insert(QStringLiteral("conversationId"),
                  message.metadata.conversationId.startsWith(QLatin1String("group:")) ? message.metadata.conversationId : QString{});
    object.insert(QStringLiteral("kind"), Domain::messageKindName(Domain::messageKind(message)));
    object.insert(QStringLiteral("payload"), Domain::messagePayloadToJson(message.payload));
    return frame(object);
}

QByteArray
Network::WireProtocol::groupSnapshotFrame(const LocalIdentity &identity, quint16 senderPort, const PeerEndpoint &recipient, const GroupSnapshot &snapshot)
{
    QJsonObject object = envelope(identity, senderPort, recipient, QStringLiteral("group.snapshot"), QDateTime::currentDateTimeUtc());
    object.insert(QStringLiteral("groupId"), snapshot.groupId);
    object.insert(QStringLiteral("name"), snapshot.name);
    object.insert(QStringLiteral("ownerId"), snapshot.ownerId);
    object.insert(QStringLiteral("revision"), static_cast<qint64>(snapshot.revision));
    object.insert(QStringLiteral("createdAt"), snapshot.createdAt.toUTC().toString(Qt::ISODateWithMs));

    QJsonArray members;
    for (const GroupMemberInfo &member : snapshot.members)
    {
        members.append(QJsonObject{
            {QStringLiteral("peerId"), member.peerId}, {QStringLiteral("displayName"), member.displayName}, {QStringLiteral("owner"), member.owner}});
    }
    object.insert(QStringLiteral("members"), members);
    return frame(object);
}

QByteArray
Network::WireProtocol::attachmentHeaderFrame(const LocalIdentity &identity, quint16 senderPort, const PeerEndpoint &recipient, const Domain::Message &message)
{
    QJsonObject object = envelope(identity, senderPort, recipient, QStringLiteral("attachment"), message.metadata.timestamp);
    object.insert(QStringLiteral("messageId"), message.metadata.messageId);
    object.insert(QStringLiteral("conversationId"), QString{});
    object.insert(QStringLiteral("kind"), Domain::messageKindName(Domain::messageKind(message)));
    object.insert(QStringLiteral("payload"), Domain::messagePayloadToJson(message.payload));
    return frame(object);
}

QByteArray
Network::WireProtocol::attachmentAcceptanceFrame(const LocalIdentity &identity, quint16 senderPort, const PeerEndpoint &recipient, const QString &transferId)
{
    QJsonObject object = envelope(identity, senderPort, recipient, QStringLiteral("attachment.accept"), QDateTime::currentDateTimeUtc());
    object.insert(QStringLiteral("messageId"), transferId);
    return frame(object);
}

bool Network::WireProtocol::isEnvelopeFor(const QJsonObject &object, const LocalIdentity &identity)
{
    return object.value(QStringLiteral("app")).toString() == QLatin1String(ProtocolName) &&
           object.value(QStringLiteral("version")).toInt() == ProtocolVersion && object.value(QStringLiteral("recipientId")).toString() == identity.deviceId;
}

Network::PeerEndpoint Network::WireProtocol::senderFromEnvelope(const QJsonObject &object, const QHostAddress &address)
{
    PeerEndpoint peer;
    peer.peerId = object.value(QStringLiteral("senderId")).toString();
    peer.displayName = object.value(QStringLiteral("senderName")).toString().trimmed().left(64);
    peer.address = address;
    const int port = object.value(QStringLiteral("senderPort")).toInt();
    if (port > 0 && port <= 65535)
    {
        peer.tcpPort = static_cast<quint16>(port);
    }
    return peer;
}
