/**
 * @file wireprotocol.h
 * @brief 声明版本 3 消息、群组快照与附件帧编码辅助函数。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef WIREPROTOCOL_H
#define WIREPROTOCOL_H

#include "domain/chattypes.h"

#include <QByteArray>
#include <QJsonObject>

namespace Network::WireProtocol
{

inline constexpr int ProtocolVersion = 3;
inline constexpr quint32 MaximumFrameSize = 64 * 1024;
inline constexpr qint64 MaximumFileSize = 2LL * 1024 * 1024 * 1024;

/**
 * @brief 构造统一消息帧。
 * @param identity 发送方身份。
 * @param senderPort 发送方监听端口。
 * @param recipient 目标节点。
 * @param message 待发送消息。
 * @return 已编码的长度前缀帧。
 */
[[nodiscard]] QByteArray messageFrame(const LocalIdentity &identity,
                                      quint16 senderPort,
                                      const PeerEndpoint &recipient,
                                      const Domain::Message &message);

/**
 * @brief 构造完整群组快照帧。
 * @param identity 发送方身份。
 * @param senderPort 发送方监听端口。
 * @param recipient 目标节点。
 * @param snapshot 群组快照。
 * @return 已编码的长度前缀帧。
 */
[[nodiscard]] QByteArray groupSnapshotFrame(
    const LocalIdentity &identity,
    quint16 senderPort,
    const PeerEndpoint &recipient,
    const GroupSnapshot &snapshot);

/**
 * @brief 构造附件传输头帧。
 * @param identity 发送方身份。
 * @param senderPort 发送方监听端口。
 * @param recipient 目标节点。
 * @param message 附件消息。
 * @return 已编码的长度前缀帧。
 */
[[nodiscard]] QByteArray attachmentHeaderFrame(const LocalIdentity &identity,
                                               quint16 senderPort,
                                               const PeerEndpoint &recipient,
                                               const Domain::Message &message);

/**
 * @brief 判断信封是否为当前版本且发送给指定本地身份。
 * @param object 已解析的信封对象。
 * @param identity 本地身份。
 * @return 信封有效且目标匹配时返回 @c true。
 */
[[nodiscard]] bool isEnvelopeFor(const QJsonObject &object,
                                 const LocalIdentity &identity);

/**
 * @brief 从信封解析发送方节点信息。
 * @param object 已解析的信封对象。
 * @param address 数据连接对应的网络地址。
 * @return 解析并补全后的发送方节点信息。
 */
[[nodiscard]] PeerEndpoint senderFromEnvelope(const QJsonObject &object,
                                              const QHostAddress &address);

} // namespace Network::WireProtocol

#endif // WIREPROTOCOL_H
