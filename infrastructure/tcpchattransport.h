/**
 * @file tcpchattransport.h
 * @brief 声明基于 TCP 的聊天与附件传输实现。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef TCPCHATTRANSPORT_H
#define TCPCHATTRANSPORT_H

#include "domain/ichattransport.h"

#include <QHash>
#include <QQueue>
#include <QSet>
#include <QTcpServer>
#include <QTimer>

class QJsonObject;
class QTcpSocket;

class TcpChatTransport final : public IChatTransport
{
    Q_OBJECT

public:
    /**
     * @brief 构造 TCP 聊天传输对象。
     * @param parent 可选的 QObject 父对象。
     */
    explicit TcpChatTransport(QObject *parent = nullptr);
    /** @brief 停止活动传输并销毁 TCP 聊天传输对象。 */
    ~TcpChatTransport() override;

    /**
     * @brief 启动 TCP 监听服务。
     * @param identity 本地身份信息。
     * @return 监听服务启动成功时返回 @c true。
     */
    [[nodiscard]] bool start(const Network::LocalIdentity &identity) override;
    /** @brief 停止监听并关闭全部活动连接。 */
    void stop() override;
    /**
     * @brief 更新后续消息携带的本地身份。
     * @param identity 新的本地身份信息。
     */
    void updateIdentity(const Network::LocalIdentity &identity) override;
    /**
     * @brief 返回 TCP 传输服务是否正在运行。
     * @return 传输服务正在运行时返回 @c true。
     */
    [[nodiscard]] bool isRunning() const override;
    /**
     * @brief 返回当前监听的 TCP 端口。
     * @return 当前监听端口；未运行时返回 @c 0。
     */
    [[nodiscard]] quint16 listeningPort() const override;
    /**
     * @brief 返回最近一次 TCP 传输错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] QString lastError() const override;

    /**
     * @brief 向指定节点发送消息。
     * @param peer 目标节点。
     * @param message 待发送消息。
     * @param[out] errorMessage 请求被拒绝时接收错误说明；允许为空。
     * @return 请求被接受时返回 @c true。
     */
    [[nodiscard]] bool sendMessage(const Network::PeerEndpoint &peer, const Domain::Message &message, QString *errorMessage = nullptr) override;
    /**
     * @brief 向指定节点发送完整群组快照。
     * @param peer 目标节点。
     * @param snapshot 群组元数据与成员快照。
     */
    void sendGroupSnapshot(const Network::PeerEndpoint &peer, const Network::GroupSnapshot &snapshot) override;
    /**
     * @brief 取消正在进行的文件传输。
     * @param peerId 远端节点标识。
     * @param transferId 文件传输标识。
     * @return 成功取消匹配的传输时返回 @c true。
     */
    [[nodiscard]] bool cancelFileTransfer(const QString &peerId, const QString &transferId) override;
    /**
     * @brief 接受等待确认的文件传输请求并通知发送方开始发送。
     * @param peerId 发送方节点标识。
     * @param transferId 文件传输标识。
     * @param[out] errorMessage 接受失败时接收错误说明；允许为空。
     * @return 已开始接收匹配文件时返回 @c true。
     */
    [[nodiscard]] bool acceptFileTransfer(const QString &peerId, const QString &transferId, QString *errorMessage = nullptr) override;

private:
    static constexpr qint64 FileTransferTimeoutMs = 30000;

    struct OutgoingFileTransfer;
    struct IncomingFileTransfer;

    /** @brief 接收服务器上等待处理的 TCP 连接。 */
    void acceptConnections();
    /**
     * @brief 读取并解析指定套接字中的数据。
     * @param socket 产生可读数据的套接字。
     */
    void readIncomingData(QTcpSocket *socket);
    /**
     * @brief 处理收到的消息帧。
     * @param object 已解析的消息对象。
     * @param socket 消息来源套接字。
     */
    void handleIncomingMessage(const QJsonObject &object, QTcpSocket *socket);
    /**
     * @brief 处理收到的群组快照帧。
     * @param object 已解析的群组快照对象。
     * @param socket 消息来源套接字。
     */
    void handleIncomingGroupSnapshot(const QJsonObject &object, QTcpSocket *socket);
    /**
     * @brief 处理收到的附件头帧。
     * @param object 已解析的附件头对象。
     * @param socket 附件来源套接字。
     * @return 附件接收上下文创建成功时返回 @c true。
     */
    bool handleIncomingAttachmentHeader(const QJsonObject &object, QTcpSocket *socket);
    /**
     * @brief 将缓冲区中的文件数据写入接收文件。
     * @param socket 文件来源套接字。
     * @param[in,out] buffer 待读取并消费的数据缓冲区。
     * @return 当前文件传输仍可继续时返回 @c true。
     */
    bool consumeIncomingFile(QTcpSocket *socket, QByteArray &buffer);
    /**
     * @brief 为已确认的接收任务创建并打开本地目标文件。
     * @param transfer 待准备的接收任务。
     * @param[out] errorMessage 准备失败时接收错误说明；允许为空。
     * @return 目标文件已准备好时返回 @c true。
     */
    bool prepareIncomingFile(IncomingFileTransfer *transfer, QString *errorMessage = nullptr);
    /**
     * @brief 结束失败或取消的文件接收任务。
     * @param socket 文件来源套接字。
     * @param reason 失败或取消原因。
     * @param cancelled 是否由取消操作触发。
     */
    void failIncomingFile(QTcpSocket *socket, const QString &reason, bool cancelled = false);

    /**
     * @brief 完成短连接事件发送任务。
     * @param socket 事件使用的套接字。
     */
    void finishOutgoingText(QTcpSocket *socket);
    /**
     * @brief 结束失败的短连接事件发送任务。
     * @param socket 事件使用的套接字。
     * @param reason 发送失败原因。
     */
    void failOutgoingText(QTcpSocket *socket, const QString &reason);
    /**
     * @brief 创建短连接并发送一个已编码事件帧。
     * @param peer 目标节点。
     * @param eventId 消息或群组标识。
     * @param frameType 事件类型。
     * @param frame 已编码帧。
     */
    void sendFramedEvent(const Network::PeerEndpoint &peer, const QString &eventId, const QString &frameType, const QByteArray &frame);
    /**
     * @brief 开始发送消息携带的本地附件。
     * @param peer 目标节点。
     * @param message 附件消息。
     * @param[out] errorMessage 请求被拒绝时接收错误说明；允许为空。
     * @return 请求被接受时返回 @c true。
     */
    [[nodiscard]] bool sendAttachment(const Network::PeerEndpoint &peer, const Domain::Message &message, QString *errorMessage);
    /**
     * @brief 向套接字持续写入待发送的文件数据。
     * @param socket 文件发送使用的套接字。
     */
    void pumpOutgoingFile(QTcpSocket *socket);
    /**
     * @brief 读取接收方通过当前发送连接返回的文件接收确认。
     * @param socket 文件发送使用的套接字。
     */
    void readOutgoingFileResponse(QTcpSocket *socket);
    /**
     * @brief 完成文件发送任务。
     * @param socket 文件发送使用的套接字。
     */
    void finishOutgoingFile(QTcpSocket *socket);
    /**
     * @brief 结束失败或取消的文件发送任务。
     * @param socket 文件发送使用的套接字。
     * @param reason 失败或取消原因。
     * @param cancelled 是否由取消操作触发。
     */
    void failOutgoingFile(QTcpSocket *socket, const QString &reason, bool cancelled = false);
    /** @brief 终止超过允许空闲时间的文件传输。 */
    void expireFileTransfers();

    /**
     * @brief 从线协议信封中解析发送方节点。
     * @param object 已解析的信封对象。
     * @param socket 消息来源套接字。
     * @return 解析并补全后的发送方节点信息。
     */
    [[nodiscard]] Network::PeerEndpoint incomingPeer(const QJsonObject &object, QTcpSocket *socket) const;
    /**
     * @brief 记录事件标识并过滤重复事件。
     * @param eventId 待记录的事件标识。
     * @return 事件标识首次出现时返回 @c true。
     */
    [[nodiscard]] bool rememberEventId(const QString &eventId);
    /**
     * @brief 为接收文件生成不冲突的本地路径。
     * @param fileName 远端提供的文件名。
     * @return 可用于保存文件的唯一路径。
     */
    [[nodiscard]] QString uniqueReceivePath(const QString &fileName) const;
    /**
     * @brief 从 JSON 对象解析时间戳。
     * @param object 包含时间戳字段的 JSON 对象。
     * @return 解析后的时间戳。
     */
    static QDateTime timestampFrom(const QJsonObject &object);
    /**
     * @brief 更新最近一次传输错误。
     * @param error 新的错误文本；传入空字符串表示清除错误。
     */
    void setLastError(const QString &error);

    QTcpServer m_server;
    QTimer m_transferTimer;
    QHash<QTcpSocket *, QByteArray> m_incomingBuffers;
    QHash<QTcpSocket *, IncomingFileTransfer *> m_incomingFiles;
    QHash<QTcpSocket *, OutgoingFileTransfer *> m_outgoingFiles;
    QSet<QTcpSocket *> m_outgoingTextSockets;
    QSet<QString> m_receivedEventIds;
    QQueue<QString> m_receivedEventOrder;
    Network::LocalIdentity m_identity;
    QString m_lastError;
    bool m_running = false;
};

#endif // TCPCHATTRANSPORT_H
