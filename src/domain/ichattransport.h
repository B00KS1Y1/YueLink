/**
 * @file ichattransport.h
 * @brief 声明统一消息、群组快照与附件传输通信抽象接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef ICHATTRANSPORT_H
#define ICHATTRANSPORT_H

#include "chattypes.h"

#include <QObject>

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
     * @brief 使用指定身份启动监听与发送功能。
     * @param identity 向远端节点公布的本地身份。
     * @return 传输服务启动成功时返回 @c true。
     */
    [[nodiscard]] virtual bool start(const Network::LocalIdentity &identity) = 0;

    /** @brief 停止传输服务并释放活动连接。 */
    virtual void stop() = 0;
    /**
     * @brief 更新后续帧携带的本地身份。
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
     * @brief 向指定节点发送消息。
     * @param peer 目标节点。
     * @param message 待发送的领域消息；附件消息需包含本地路径。
     * @param[out] errorMessage 请求被拒绝时接收错误说明；允许为空。
     * @return 发送请求被接受时返回 @c true。
     */
    [[nodiscard]] virtual bool sendMessage(const Network::PeerEndpoint &peer,
                                           const Domain::Message &message,
                                           QString *errorMessage = nullptr) = 0;

    /**
     * @brief 向指定节点发送完整群组快照。
     * @param peer 目标节点。
     * @param snapshot 群组元数据与成员快照。
     */
    virtual void sendGroupSnapshot(const Network::PeerEndpoint &peer, const Network::GroupSnapshot &snapshot) = 0;

    /**
     * @brief 取消正在进行的附件传输。
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
     * @brief 收到消息时发出。
     * @param message 收到的领域消息。
     * @param sender 发送方端点。
     */
    void messageReceived(const Domain::Message &message,
                         const Network::PeerEndpoint &sender);

    /**
     * @brief 收到群组快照时发出。
     * @param snapshot 已验证的群组快照。
     */
    void groupSnapshotReceived(const Network::GroupSnapshot &snapshot);

    /**
     * @brief 消息写入套接字后发出。
     * @param peerId 目标节点标识。
     * @param messageId 消息标识。
     */
    void messageSent(const QString &peerId, const QString &messageId);

    /**
     * @brief 消息发送失败时发出。
     * @param peerId 目标节点标识。
     * @param messageId 消息标识。
     * @param reason 失败原因。
     */
    void messageSendFailed(const QString &peerId, const QString &messageId, const QString &reason);

    /**
     * @brief 群组快照发送失败时发出。
     * @param peerId 目标节点标识。
     * @param groupId 群组标识。
     * @param reason 失败原因。
     */
    void groupSnapshotSendFailed(const QString &peerId, const QString &groupId, const QString &reason);

    /**
     * @brief 附件传输开始时发出。
     * @param transfer 附件传输元数据。
     */
    void attachmentTransferStarted(const Domain::AttachmentTransferInfo &transfer);

    /**
     * @brief 附件传输进度变化时发出。
     * @param progress 附件传输进度事件。
     */
    void attachmentTransferProgressed(const Domain::AttachmentTransferProgress &progress);

    /**
     * @brief 附件传输完成、失败或取消时发出。
     * @param result 最终传输结果。
     */
    void attachmentTransferFinished(const Domain::AttachmentTransferResult &result);

    /**
     * @brief 传输服务报告一般性错误时发出。
     * @param message 错误说明。
     */
    void errorOccurred(const QString &message);
};

#endif // ICHATTRANSPORT_H
