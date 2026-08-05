/**
 * @file networktypes.h
 * @brief 定义网络身份、群组快照、消息及文件传输事件类型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef NETWORKTYPES_H
#define NETWORKTYPES_H

#include <QDateTime>
#include <QHostAddress>
#include <QList>
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

    /**
     * @brief 检查节点是否包含完整的路由信息。
     * @return 节点标识、名称、地址和端口均有效时返回 @c true。
     */
    [[nodiscard]] bool isValid() const
    {
        return !peerId.isEmpty() && !displayName.isEmpty() && !address.isNull()
               && tcpPort != 0;
    }
};

struct TextMessage
{
    QString messageId;
    QString groupId;
    PeerEndpoint sender;
    QString text;
    QDateTime timestamp;
};

struct GroupMemberInfo
{
    QString peerId;
    QString displayName;
    bool owner = false;
};

struct GroupSnapshot
{
    QString groupId;
    QString name;
    QString ownerId;
    quint64 revision = 1;
    QDateTime createdAt;
    PeerEndpoint sender;
    QList<GroupMemberInfo> members;
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
Q_DECLARE_METATYPE(Network::GroupSnapshot)
Q_DECLARE_METATYPE(Network::TransferDirection)
Q_DECLARE_METATYPE(Network::FileTransferInfo)
Q_DECLARE_METATYPE(Network::FileTransferProgress)
Q_DECLARE_METATYPE(Network::FileTransferResult)

#endif // NETWORKTYPES_H
