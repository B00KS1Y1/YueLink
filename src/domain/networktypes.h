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
    QString deviceId;    ///< 本机设备标识。
    QString displayName; ///< 本机显示名称。
};

struct PeerEndpoint
{
    QString peerId;       ///< 对端设备标识。
    QString displayName;  ///< 对端显示名称。
    QHostAddress address; ///< 对端网络地址。
    quint16 tcpPort = 0;  ///< 对端 TCP 端口。

    /**
     * @brief 检查节点是否包含完整的路由信息。
     * @return 节点标识、名称、地址和端口均有效时返回 @c true。
     */
    [[nodiscard]] bool isValid() const
    {
        return !peerId.isEmpty() && !displayName.isEmpty() && !address.isNull() && tcpPort != 0;
    }
};

struct TextMessage
{
    QString messageId;   ///< 消息标识。
    QString groupId;     ///< 群组标识；为空表示直聊。
    PeerEndpoint sender; ///< 发送方端点。
    QString text;        ///< 文本内容。
    QDateTime timestamp; ///< 发送时间。
};

struct GroupMemberInfo
{
    QString peerId;      ///< 成员设备标识。
    QString displayName; ///< 成员显示名称。
    bool owner = false;  ///< 是否为群主。
};

struct GroupSnapshot
{
    QString groupId;                ///< 群组标识。
    QString name;                   ///< 群组名称。
    QString ownerId;                ///< 群主设备标识。
    quint64 revision = 1;           ///< 快照版本号。
    QDateTime createdAt;            ///< 创建时间。
    PeerEndpoint sender;            ///< 快照发送方。
    QList<GroupMemberInfo> members; ///< 群成员列表。
};

/**
 * @brief 文件传输方向。
 */
enum class TransferDirection
{
    Outgoing, ///< 发送文件。
    Incoming  ///< 接收文件。
};

struct FileTransferInfo
{
    QString transferId;                                        ///< 传输标识。
    PeerEndpoint peer;                                         ///< 远端节点。
    QString fileName;                                          ///< 文件名。
    QString filePath;                                          ///< 本地文件路径。
    qint64 fileSize = 0;                                       ///< 文件大小（字节）。
    QDateTime timestamp;                                       ///< 发起时间。
    TransferDirection direction = TransferDirection::Outgoing; ///< 传输方向。
};

struct FileTransferProgress
{
    QString peerId;                                            ///< 远端设备标识。
    QString transferId;                                        ///< 传输标识。
    TransferDirection direction = TransferDirection::Outgoing; ///< 传输方向。
    qreal progress = 0.0;                                      ///< 传输进度，范围为 0.0～1.0。
};

struct FileTransferResult
{
    QString peerId;                                            ///< 远端设备标识。
    QString transferId;                                        ///< 传输标识。
    QString filePath;                                          ///< 本地文件路径。
    QString errorMessage;                                      ///< 错误或取消原因。
    TransferDirection direction = TransferDirection::Outgoing; ///< 传输方向。
    bool success = false;                                      ///< 是否成功。
    bool cancelled = false;                                    ///< 是否取消。
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
