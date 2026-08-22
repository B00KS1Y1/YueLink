/**
 * @file sqlitechatrepository.h
 * @brief 声明 SQLite 统一会话、群组、消息与逐成员投递仓储。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef SQLITECHATREPOSITORY_H
#define SQLITECHATREPOSITORY_H

#include "domain/ichatrepository.h"

#include <QSqlDatabase>
#include <QString>

class SqliteChatRepository final : public IChatRepository
{
public:
    /** @brief 构造使用独立连接名的 SQLite 仓储。 */
    SqliteChatRepository();
    /** @brief 关闭数据库连接并销毁仓储。 */
    ~SqliteChatRepository() override;

    /**
     * @brief 打开数据库并在版本不匹配时清空开发数据后重建架构。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 初始化成功时返回 @c true。
     */
    [[nodiscard]] bool initialize(QString *errorMessage) override;
    /**
     * @brief 加载联系人。
     * @param[out] peers 接收联系人。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] bool loadPeers(QList<Domain::Peer> *peers, QString *errorMessage) override;
    /**
     * @brief 加载会话摘要。
     * @param[out] conversations 接收会话摘要。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] bool loadConversations(QList<Domain::Conversation> *conversations, QString *errorMessage) override;
    /**
     * @brief 加载群组及成员。
     * @param[out] groups 接收群组。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] bool loadGroups(QList<Domain::Group> *groups, QString *errorMessage) override;
    /**
     * @brief 加载指定会话的最近消息。
     * @param conversationId 会话标识。
     * @param limit 最大消息数量。
     * @param[out] messages 接收消息。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] bool loadMessages(const QString &conversationId, int limit, QList<Domain::Message> *messages, QString *errorMessage) override;
    /**
     * @brief 根据标识加载单条消息。
     * @param messageId 消息标识。
     * @param[out] message 接收消息。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 找到消息时返回 @c true。
     */
    [[nodiscard]] bool loadMessage(const QString &messageId, Domain::Message *message, QString *errorMessage) override;
    /**
     * @brief 加载全部逐成员投递。
     * @param[out] deliveries 接收投递记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 加载成功时返回 @c true。
     */
    [[nodiscard]] bool loadDeliveries(QList<Domain::MessageDelivery> *deliveries, QString *errorMessage) override;

    /**
     * @brief 新增或更新联系人。
     * @param peer 联系人记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] bool upsertPeer(const Domain::Peer &peer, QString *errorMessage) override;
    /**
     * @brief 新增或更新会话。
     * @param conversation 会话记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] bool saveConversation(const Domain::Conversation &conversation, QString *errorMessage) override;
    /**
     * @brief 原子保存群组及完整成员快照。
     * @param group 群组记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] bool saveGroup(const Domain::Group &group, QString *errorMessage) override;
    /**
     * @brief 更新会话摘要和未读数。
     * @param conversationId 会话标识。
     * @param lastMessage 最新消息摘要。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读数。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] bool updateConversation(
        const QString &conversationId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread, QString *errorMessage) override;
    /**
     * @brief 清空会话未读数。
     * @param conversationId 会话标识。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] bool clearUnread(const QString &conversationId, QString *errorMessage) override;
    /**
     * @brief 更新会话置顶状态。
     * @param conversationId 会话标识。
     * @param pinned 是否置顶。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] bool setConversationPinned(const QString &conversationId, bool pinned, QString *errorMessage) override;
    /**
     * @brief 原子删除会话消息与投递记录并隐藏会话。
     * @param conversationId 会话标识。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 删除并隐藏成功时返回 @c true。
     */
    [[nodiscard]] bool removeConversation(const QString &conversationId, QString *errorMessage) override;
    /**
     * @brief 新增或更新消息。
     * @param message 消息记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] bool saveMessage(const Domain::Message &message, QString *errorMessage) override;
    /**
     * @brief 更新消息状态。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param state 新状态。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] bool updateMessageState(const QString &conversationId, const QString &messageId, Domain::DeliveryState state, QString *errorMessage) override;
    /**
     * @brief 更新文件消息传输状态。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param progress 传输进度。
     * @param state 新状态。
     * @param filePath 可用时提供本地路径。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 更新成功时返回 @c true。
     */
    [[nodiscard]] bool updateFileTransfer(const QString &conversationId,
                                          const QString &messageId,
                                          qreal progress,
                                          Domain::DeliveryState state,
                                          const QString &filePath,
                                          QString *errorMessage) override;
    /**
     * @brief 新增或更新逐成员投递记录。
     * @param delivery 投递记录。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 保存成功时返回 @c true。
     */
    [[nodiscard]] bool saveDelivery(const Domain::MessageDelivery &delivery, QString *errorMessage) override;

private:
    /**
     * @brief 返回当前独立数据库连接。
     * @return SQLite 数据库连接。
     */
    [[nodiscard]] QSqlDatabase database() const;
    /**
     * @brief 应用 busy timeout、WAL、外键与同步模式配置。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 配置成功时返回 @c true。
     */
    [[nodiscard]] bool configureDatabase(QString *errorMessage);
    /**
     * @brief 检查架构版本并按需清空后重建全部聊天表。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 架构可用时返回 @c true。
     */
    [[nodiscard]] bool ensureSchema(QString *errorMessage);
    /**
     * @brief 执行单条无参数 SQL。
     * @param statement SQL 文本。
     * @param[out] errorMessage 失败时接收错误说明。
     * @return 执行成功时返回 @c true。
     */
    [[nodiscard]] bool executeStatement(const QString &statement, QString *errorMessage);

    QString m_connectionName;
    QString m_databasePath;
    bool m_initialized = false;
};

#endif // SQLITECHATREPOSITORY_H
