#ifndef NETWORKTYPES_H
#define NETWORKTYPES_H

#include <QDateTime>
#include <QHostAddress>
#include <QMetaType>
#include <QString>

namespace Network
{

struct LocalIdentity
{
    QString deviceId;
    QString displayName;
};

struct PeerEndpoint
{
    QString peerId;
    QString displayName;
    QHostAddress address;
    quint16 tcpPort = 0;

    [[nodiscard]] bool isValid() const
    {
        return !peerId.isEmpty() && !displayName.isEmpty() && !address.isNull() && tcpPort != 0;
    }
};

struct TextMessage
{
    QString messageId;
    PeerEndpoint sender;
    QString text;
    QDateTime timestamp;
};

enum class TransferDirection
{
    Outgoing,
    Incoming
};

struct FileTransferInfo
{
    QString transferId;
    PeerEndpoint peer;
    QString fileName;
    QString filePath;
    qint64 fileSize = 0;
    QDateTime timestamp;
    TransferDirection direction = TransferDirection::Outgoing;
};

struct FileTransferProgress
{
    QString peerId;
    QString transferId;
    TransferDirection direction = TransferDirection::Outgoing;
    qreal progress = 0.0;
};

struct FileTransferResult
{
    QString peerId;
    QString transferId;
    QString filePath;
    QString errorMessage;
    TransferDirection direction = TransferDirection::Outgoing;
    bool success = false;
    bool cancelled = false;
};

} // namespace Network

Q_DECLARE_METATYPE(Network::PeerEndpoint)
Q_DECLARE_METATYPE(Network::TextMessage)
Q_DECLARE_METATYPE(Network::TransferDirection)
Q_DECLARE_METATYPE(Network::FileTransferInfo)
Q_DECLARE_METATYPE(Network::FileTransferProgress)
Q_DECLARE_METATYPE(Network::FileTransferResult)

#endif // NETWORKTYPES_H
