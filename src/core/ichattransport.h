/**
 * @file ichattransport.h
 * @brief 声明聊天消息与文件传输的通信抽象接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef ICHATTRANSPORT_H
#define ICHATTRANSPORT_H

#include "networktypes.h"

#include <QDateTime>
#include <QObject>
#include <QUrl>

class IChatTransport : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造聊天传输对象。
     * @param parent 可选的 QObject 父对象。
     */
    explicit IChatTransport(QObject *parent = nullptr)
    : QObject(parent)
    {
    }

    /** @brief 销毁聊天传输对象。 */
    ~IChatTransport() override = default;

    /**
     * @brief 使用指定身份启动监听与消息发送功能。
     * @param identity 向远端节点公布的本地身份。
     * @return 传输服务启动成功时返回 @c true。
     */
    [[nodiscard]] virtual bool start(const Network::LocalIdentity &identity) = 0;
    /** @brief 停止传输服务并释放活动连接。 */
    virtual void stop() = 0;
    /**
     * @brief 更新后续消息携带的本地身份。
     * @param identity 新的本地身份。
     */
    virtual void updateIdentity(const Network::LocalIdentity &identity) = 0;
    /**
     * @brief 返回传输服务是否正在运行。
     * @return 传输服务正在运行时返回 @c true。
     */
    [[nodiscard]] virtual bool isRunning() const = 0;
    /**
     * @brief 返回当前接收连接的 TCP 端口。
     * @return 当前监听端口；未运行时返回 @c 0。
     */
    [[nodiscard]] virtual quint16 listeningPort() const = 0;
    /**
     * @brief 返回最近一次传输错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] virtual QString lastError() const = 0;

    /**
     * @brief 向指定节点发送文本消息。
     * @param peer 目标节点。
     * @param messageId 唯一消息标识。
     * @param text 消息内容。
     * @param timestamp 消息创建时间。
     */
    virtual void sendText(const Network::PeerEndpoint &peer, const QString &messageId, const QString &text, const QDateTime &timestamp) = 0;
    /**
     * @brief 开始向指定节点发送文件。
     * @param peer 目标节点。
     * @param fileUrl 本地文件 URL。
     * @param[out] errorMessage 操作失败时接收错误说明。
     * @return 文件传输请求被接受时返回 @c true。
     */
    [[nodiscard]] virtual bool sendFile(const Network::PeerEndpoint &peer, const QUrl &fileUrl, QString *errorMessage) = 0;
    /**
     * @brief 取消正在进行的文件传输。
     * @param peerId 远端节点标识。
     * @param transferId 文件传输标识。
     * @return 成功取消匹配的传输时返回 @c true。
     */
    [[nodiscard]] virtual bool cancelFileTransfer(const QString &peerId, const QString &transferId) = 0;

signals:
    /**
     * @brief 发现或刷新节点信息时发出。
     * @param peer 已观察到的节点信息。
     */
    void peerObserved(const Network::PeerEndpoint &peer);
    /**
     * @brief 收到文本消息时发出。
     * @param message 收到的文本消息。
     */
    void textReceived(const Network::TextMessage &message);
    /**
     * @brief 文本消息写入套接字后发出。
     * @param peerId 目标节点标识。
     * @param messageId 消息标识。
     */
    void textSent(const QString &peerId, const QString &messageId);
    /**
     * @brief 文本消息发送失败时发出。
     * @param peerId 目标节点标识。
     * @param messageId 消息标识。
     * @param reason 失败原因。
     */
    void textSendFailed(const QString &peerId, const QString &messageId, const QString &reason);
    /**
     * @brief 文件传输开始时发出。
     * @param transfer 文件传输元数据。
     */
    void fileTransferStarted(const Network::FileTransferInfo &transfer);
    /**
     * @brief 文件传输进度变化时发出。
     * @param progress 文件传输进度事件。
     */
    void fileTransferProgressed(const Network::FileTransferProgress &progress);
    /**
     * @brief 文件传输完成、失败或取消时发出。
     * @param result 最终传输结果。
     */
    void fileTransferFinished(const Network::FileTransferResult &result);
    /**
     * @brief 传输服务报告一般性错误时发出。
     * @param message 错误说明。
     */
    void errorOccurred(const QString &message);
};

#endif // ICHATTRANSPORT_H
