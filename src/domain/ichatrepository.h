/**
 * @file ichatrepository.h
 * @brief 声明好友与消息数据的持久化抽象接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef ICHATREPOSITORY_H
#define ICHATREPOSITORY_H

#include "networktypes.h"

#include <QDateTime>
#include <QList>
#include <QString>

namespace Storage
{

struct PeerRecord
{
    Network::PeerEndpoint endpoint;
    QString lastMessage;
    QDateTime lastActivity;
    int unreadCount = 0;
};

struct MessageRecord
{
    QString messageId;
    QString peerId;
    QString senderInitial;
    QString senderColor;
    QString text;
    QDateTime timestamp;
    QString deliveryStatus;
    QString messageKind = QStringLiteral("text");
    QString fileName;
    QString fileSizeText;
    QString filePath;
    qint64 fileSize = 0;
    qreal fileProgress = 0.0;
    bool fromMe = false;
};

} // namespace Storage

class IChatRepository
{
public:
    /** @brief 销毁聊天数据仓储对象。 */
    virtual ~IChatRepository() = default;

    /**
     * @brief 初始化数据仓储并执行必要的数据迁移。
     * @param[out] errorMessage 初始化失败时接收错误说明。
     * @return 数据仓储可用时返回 @c true。
     */
    [[nodiscard]] virtual bool initialize(QString *errorMessage) = 0;
    /**
     * @brief 加载已持久化的节点摘要。
     * @param[out] peers 接收节点记录。
     * @param[out] errorMessage 加载失败时接收错误说明。
     * @return 记录加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool loadPeers(QList<Storage::PeerRecord> *peers,
                                         QString *errorMessage) = 0;
    /**
     * @brief 加载指定节点的最近消息。
     * @param peerId 待加载会话的节点标识。
     * @param limit 最多返回的记录数量。
     * @param[out] messages 接收消息记录。
     * @param[out] errorMessage 加载失败时接收错误说明。
     * @return 记录加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool loadMessages(const QString &peerId,
                                            int limit,
                                            QList<Storage::MessageRecord> *messages,
                                            QString *errorMessage) = 0;

    /**
     * @brief 新增或更新节点信息。
     * @param peer 待持久化的节点信息。
     * @param[out] errorMessage 保存失败时接收错误说明。
     * @return 节点保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool upsertPeer(const Network::PeerEndpoint &peer,
                                          QString *errorMessage) = 0;
    /**
     * @brief 更新会话摘要。
     * @param peerId 会话对应的节点标识。
     * @param lastMessage 最新消息预览。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读计数。
     * @param[out] errorMessage 更新失败时接收错误说明。
     * @return 摘要更新成功时返回 @c true。
     */
    [[nodiscard]] virtual bool updateConversation(const QString &peerId,
                                                  const QString &lastMessage,
                                                  const QDateTime &timestamp,
                                                  bool incrementUnread,
                                                  QString *errorMessage) = 0;
    /**
     * @brief 清空会话的未读计数。
     * @param peerId 会话对应的节点标识。
     * @param[out] errorMessage 清空失败时接收错误说明。
     * @return 未读计数清空成功时返回 @c true。
     */
    [[nodiscard]] virtual bool clearUnread(const QString &peerId,
                                           QString *errorMessage) = 0;

    /**
     * @brief 持久化消息记录。
     * @param message 待保存的消息记录。
     * @param[out] errorMessage 保存失败时接收错误说明。
     * @return 消息保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool saveMessage(const Storage::MessageRecord &message,
                                           QString *errorMessage) = 0;
    /**
     * @brief 更新消息的投递状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param status 新的投递状态。
     * @param[out] errorMessage 更新失败时接收错误说明。
     * @return 状态更新成功时返回 @c true。
     */
    [[nodiscard]] virtual bool updateDeliveryStatus(const QString &peerId,
                                                    const QString &messageId,
                                                    const QString &status,
                                                    QString *errorMessage) = 0;
    /**
     * @brief 更新已持久化的文件传输状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的传输进度。
     * @param status 新的传输状态。
     * @param filePath 可用时提供本地文件路径。
     * @param[out] errorMessage 更新失败时接收错误说明。
     * @return 传输状态更新成功时返回 @c true。
     */
    [[nodiscard]] virtual bool updateFileTransfer(const QString &peerId,
                                                  const QString &messageId,
                                                  qreal progress,
                                                  const QString &status,
                                                  const QString &filePath,
                                                  QString *errorMessage) = 0;
};

#endif // ICHATREPOSITORY_H
