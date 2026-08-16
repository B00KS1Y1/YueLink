/**
 * @file ichatrepository.h
 * @brief 声明联系人、统一会话、群组、消息及投递数据仓储接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef ICHATREPOSITORY_H
#define ICHATREPOSITORY_H

#include "chattypes.h"

#include <QList>
#include <QString>

class IChatRepository
{
public:
    /** @brief 销毁聊天数据仓储对象。 */
    virtual ~IChatRepository() = default;

    /**
     * @brief 初始化数据仓储；架构版本不匹配时允许清空开发数据并重建。
     * @param[out] errorMessage 初始化失败时接收错误说明。
     * @return 数据仓储可用时返回 @c true。
     */
    [[nodiscard]] virtual bool initialize(QString *errorMessage) = 0;

    /**
     * @brief 加载全部联系人记录。
     * @param[out] peers 接收联系人记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool loadPeers(QList<Domain::Peer> *peers, QString *errorMessage) = 0;

    /**
     * @brief 加载全部会话摘要。
     * @param[out] conversations 接收会话摘要。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool loadConversations(QList<Domain::Conversation> *conversations, QString *errorMessage) = 0;

    /**
     * @brief 加载全部群组及其成员。
     * @param[out] groups 接收群组记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool loadGroups(QList<Domain::Group> *groups, QString *errorMessage) = 0;

    /**
     * @brief 加载指定会话的最近消息。
     * @param conversationId 会话标识。
     * @param limit 最多返回的消息数量。
     * @param[out] messages 接收消息记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool loadMessages(const QString &conversationId, int limit, QList<Domain::Message> *messages, QString *errorMessage) = 0;

    /**
     * @brief 根据消息标识加载单条消息。
     * @param messageId 消息标识。
     * @param[out] message 接收消息内容。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 找到消息时返回 @c true。
     */
    [[nodiscard]] virtual bool loadMessage(const QString &messageId, Domain::Message *message, QString *errorMessage) = 0;

    /**
     * @brief 加载全部逐成员投递记录。
     * @param[out] deliveries 接收投递记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool loadDeliveries(QList<Domain::MessageDelivery> *deliveries, QString *errorMessage) = 0;

    /**
     * @brief 新增或更新联系人端点。
     * @param peer 待保存的联系人。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool upsertPeer(const Domain::Peer &peer, QString *errorMessage) = 0;

    /**
     * @brief 新增或更新会话摘要。
     * @param conversation 待保存的会话。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool saveConversation(const Domain::Conversation &conversation, QString *errorMessage) = 0;

    /**
     * @brief 原子保存群组元数据及完整成员快照。
     * @param group 待保存的群组。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool saveGroup(const Domain::Group &group, QString *errorMessage) = 0;

    /**
     * @brief 更新会话摘要与未读数。
     * @param conversationId 会话标识。
     * @param lastMessage 最新消息摘要。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读计数。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] virtual bool
    updateConversation(const QString &conversationId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread, QString *errorMessage) = 0;

    /**
     * @brief 清空指定会话未读数。
     * @param conversationId 会话标识。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] virtual bool clearUnread(const QString &conversationId, QString *errorMessage) = 0;

    /**
     * @brief 持久化消息。
     * @param message 待保存的消息。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool saveMessage(const Domain::Message &message, QString *errorMessage) = 0;

    /**
     * @brief 更新消息投递状态。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param state 新状态。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] virtual bool
    updateMessageState(const QString &conversationId, const QString &messageId, Domain::DeliveryState state, QString *errorMessage) = 0;

    /**
     * @brief 更新文件消息进度、状态与可选路径。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的进度。
     * @param state 新状态。
     * @param filePath 可用时提供本地路径。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] virtual bool updateFileTransfer(const QString &conversationId,
                                                  const QString &messageId,
                                                  qreal progress,
                                                  Domain::DeliveryState state,
                                                  const QString &filePath,
                                                  QString *errorMessage) = 0;

    /**
     * @brief 新增或更新逐成员投递记录。
     * @param delivery 待保存的投递记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool saveDelivery(const Domain::MessageDelivery &delivery, QString *errorMessage) = 0;
};

#endif // ICHATREPOSITORY_H
