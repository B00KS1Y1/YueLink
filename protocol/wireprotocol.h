#ifndef WIREPROTOCOL_H
#define WIREPROTOCOL_H

#include "domain/networktypes.h"

#include <QByteArray>
#include <QJsonObject>

namespace Network::WireProtocol
{

inline constexpr quint32 MaximumFrameSize = 64 * 1024;
inline constexpr qint64 MaximumFileSize = 2LL * 1024 * 1024 * 1024;

[[nodiscard]] QByteArray textFrame(const LocalIdentity &identity,
                                   quint16 senderPort,
                                   const PeerEndpoint &recipient,
                                   const QString &messageId,
                                   const QString &text,
                                   const QDateTime &timestamp);
[[nodiscard]] QByteArray fileHeaderFrame(const LocalIdentity &identity,
                                         quint16 senderPort,
                                         const PeerEndpoint &recipient,
                                         const QString &transferId,
                                         const QString &fileName,
                                         qint64 fileSize,
                                         const QDateTime &timestamp);
[[nodiscard]] bool isEnvelopeFor(const QJsonObject &object, const LocalIdentity &identity);
[[nodiscard]] PeerEndpoint senderFromEnvelope(const QJsonObject &object, const QHostAddress &address);

} // namespace Network::WireProtocol

#endif // WIREPROTOCOL_H
