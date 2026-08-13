/**
 * @file transfercoordinator.h
 * @brief 声明文件传输命令与状态编排协调器。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef TRANSFERCOORDINATOR_H
#define TRANSFERCOORDINATOR_H

#include "domain/chattypes.h"

#include <QObject>
#include <QStringList>

class ConversationStore;
class IChatTransport;

class TransferCoordinator final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造文件传输协调器。
     * @param transport 非拥有的聊天传输对象；必须覆盖本对象生命周期。
     * @param conversations 非拥有的会话存储；必须覆盖本对象生命周期。
     * @param localIdentity 非拥有的本地身份；必须覆盖本对象生命周期。
     * @param parent 可选的 QObject 父对象。
     */
    TransferCoordinator(IChatTransport *transport, ConversationStore *conversations, const Network::LocalIdentity *localIdentity, QObject *parent = nullptr);
    /** @brief 销毁文件传输协调器。 */
    ~TransferCoordinator() override;

    /**
     * @brief 向指定在线节点发送附件消息。
     * @param peer 目标节点信息。
     * @param message 已构造的附件消息。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult sendAttachment(
        const Domain::Peer &peer, const Domain::Message &message);
    /**
     * @brief 取消正在进行的文件传输。
     * @param peerId 远端节点标识。
     * @param transferId 文件传输标识。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult cancel(const QString &peerId, const QString &transferId);
    /**
     * @brief 接受等待确认的文件传输请求。
     * @param peerId 发送方节点标识。
     * @param transferId 文件传输标识。
     * @return 包含成功或失败信息的结构化结果。
     */
    [[nodiscard]] Domain::OperationResult accept(const QString &peerId,
                                                 const QString &transferId);

signals:
    /**
     * @brief 接收文件成功保存后发出。
     * @param peerId 发送方节点标识。
     * @param filePath 接收文件的本地路径。
     */
    void fileReceived(const QString &peerId, const QString &filePath);
    /**
     * @brief 文件传输失败时发出。
     * @param peerId 远端节点标识。
     * @param reason 便于用户阅读的失败原因。
     * @param incoming 失败的传输是否为接收方向。
     */
    void fileTransferFailed(const QString &peerId, const QString &reason, bool incoming);
    /**
     * @brief 批量文件传输请求存在一般性错误时发出。
     * @param reason 便于用户阅读的失败原因。
     */
    void operationFailed(const QString &reason);

private:
    /**
     * @brief 处理文件传输开始事件并创建会话消息。
     * @param transfer 文件传输元数据。
     */
    void handleStarted(const Domain::AttachmentTransferInfo &transfer);
    /**
     * @brief 处理文件传输进度事件。
     * @param progress 文件传输进度。
     */
    void handleProgress(const Domain::AttachmentTransferProgress &progress);
    /**
     * @brief 处理文件传输完成、失败或取消事件。
     * @param result 文件传输最终结果。
     */
    void handleFinished(const Domain::AttachmentTransferResult &result);

    IChatTransport *m_transport = nullptr;
    ConversationStore *m_conversations = nullptr;
    const Network::LocalIdentity *m_localIdentity = nullptr;
};

#endif // TRANSFERCOORDINATOR_H
