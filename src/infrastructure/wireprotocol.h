/**
 * @file wireprotocol.h
 * @brief 声明聊天消息与文件传输帧的线协议辅助函数。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef WIREPROTOCOL_H
#define WIREPROTOCOL_H

#include "domain/networktypes.h"

#include <QByteArray>
#include <QJsonObject>

namespace Network::WireProtocol
{

inline constexpr quint32 MaximumFrameSize = 64 * 1024;
inline constexpr qint64 MaximumFileSize = 2LL * 1024 * 1024 * 1024;

/**
 * @brief 构造文本消息线协议帧。
 * @param identity 发送方本地身份。
 * @param senderPort 发送方监听端口。
 * @param recipient 目标节点。
 * @param messageId 唯一消息标识。
 * @param text 消息内容。
 * @param timestamp 消息创建时间。
 * @return 已编码的长度前缀消息帧。
 */
[[nodiscard]] QByteArray textFrame(const LocalIdentity &identity,
                                   quint16 senderPort,
                                   const PeerEndpoint &recipient,
                                   const QString &messageId,
                                   const QString &text,
                                   const QDateTime &timestamp);
/**
 * @brief 构造文件传输头线协议帧。
 * @param identity 发送方本地身份。
 * @param senderPort 发送方监听端口。
 * @param recipient 目标节点。
 * @param transferId 唯一文件传输标识。
 * @param fileName 文件名。
 * @param fileSize 文件字节数。
 * @param timestamp 文件传输创建时间。
 * @return 已编码的长度前缀文件头帧。
 */
[[nodiscard]] QByteArray fileHeaderFrame(const LocalIdentity &identity,
                                         quint16 senderPort,
                                         const PeerEndpoint &recipient,
                                         const QString &transferId,
                                         const QString &fileName,
                                         qint64 fileSize,
                                         const QDateTime &timestamp);
/**
 * @brief 判断线协议信封是否发送给指定本地身份。
 * @param object 已解析的信封对象。
 * @param identity 本地身份。
 * @return 信封目标与本地身份匹配时返回 @c true。
 */
[[nodiscard]] bool isEnvelopeFor(const QJsonObject &object, const LocalIdentity &identity);

/**
 * @brief 从线协议信封解析发送方节点信息。
 * @param object 已解析的信封对象。
 * @param address 数据连接对应的网络地址。
 * @return 解析并补全后的发送方节点信息。
 */
[[nodiscard]] PeerEndpoint senderFromEnvelope(const QJsonObject &object, const QHostAddress &address);

} // namespace Network::WireProtocol

#endif // WIREPROTOCOL_H
