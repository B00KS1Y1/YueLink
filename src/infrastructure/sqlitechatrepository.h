/**
 * @file sqlitechatrepository.h
 * @brief 声明聊天数据仓储的 SQLite 实现。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef SQLITECHATREPOSITORY_H
#define SQLITECHATREPOSITORY_H

#include "core/ichatrepository.h"

class QSqlDatabase;

class SqliteChatRepository final : public IChatRepository
{
public:
    /** @brief 构造 SQLite 聊天数据仓储。 */
    SqliteChatRepository();
    /** @brief 关闭数据库连接并销毁数据仓储。 */
    ~SqliteChatRepository() override;

    /**
     * @brief 初始化数据库连接并迁移数据结构。
     * @param[out] errorMessage 初始化失败时接收错误说明。
     * @return 数据仓储可用时返回 @c true。
     */
    [[nodiscard]] bool initialize(QString *errorMessage) override;
    /**
     * @brief 加载全部节点摘要。
     * @param[out] peers 接收节点记录。
     * @param[out] errorMessage 加载失败时接收错误说明。
     * @return 记录加载成功时返回 @c true。
     */
    [[nodiscard]] bool loadPeers(QList<Storage::PeerRecord> *peers, QString *errorMessage) override;
    /**
     * @brief 加载指定会话的最近消息。
     * @param peerId 会话对应的节点标识。
     * @param limit 最多返回的消息数量。
     * @param[out] messages 接收消息记录。
     * @param[out] errorMessage 加载失败时接收错误说明。
     * @return 记录加载成功时返回 @c true。
     */
    [[nodiscard]] bool loadMessages(const QString &peerId, int limit, QList<Storage::MessageRecord> *messages, QString *errorMessage) override;

    /**
     * @brief 新增或更新节点信息。
     * @param peer 待持久化的节点信息。
     * @param[out] errorMessage 保存失败时接收错误说明。
     * @return 节点保存成功时返回 @c true。
     */
    [[nodiscard]] bool upsertPeer(const Network::PeerEndpoint &peer, QString *errorMessage) override;
    /**
     * @brief 更新会话摘要及未读计数。
     * @param peerId 会话对应的节点标识。
     * @param lastMessage 最新消息预览。
     * @param timestamp 最近活动时间。
     * @param incrementUnread 是否增加未读计数。
     * @param[out] errorMessage 更新失败时接收错误说明。
     * @return 摘要更新成功时返回 @c true。
     */
    [[nodiscard]] bool
    updateConversation(const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread, QString *errorMessage) override;
    /**
     * @brief 清空指定会话的未读计数。
     * @param peerId 会话对应的节点标识。
     * @param[out] errorMessage 清空失败时接收错误说明。
     * @return 未读计数清空成功时返回 @c true。
     */
    [[nodiscard]] bool clearUnread(const QString &peerId, QString *errorMessage) override;

    /**
     * @brief 保存消息记录。
     * @param message 待保存的消息记录。
     * @param[out] errorMessage 保存失败时接收错误说明。
     * @return 消息保存成功时返回 @c true。
     */
    [[nodiscard]] bool saveMessage(const Storage::MessageRecord &message, QString *errorMessage) override;
    /**
     * @brief 更新消息投递状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param status 新的投递状态。
     * @param[out] errorMessage 更新失败时接收错误说明。
     * @return 状态更新成功时返回 @c true。
     */
    [[nodiscard]] bool updateDeliveryStatus(const QString &peerId, const QString &messageId, const QString &status, QString *errorMessage) override;
    /**
     * @brief 更新文件传输进度与状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的传输进度。
     * @param status 新的传输状态。
     * @param filePath 可用时提供本地文件路径。
     * @param[out] errorMessage 更新失败时接收错误说明。
     * @return 传输状态更新成功时返回 @c true。
     */
    [[nodiscard]] bool updateFileTransfer(
        const QString &peerId, const QString &messageId, qreal progress, const QString &status, const QString &filePath, QString *errorMessage) override;

private:
    /**
     * @brief 返回当前线程使用的数据库连接。
     * @return 当前数据仓储使用的数据库连接。
     */
    [[nodiscard]] QSqlDatabase database() const;
    /**
     * @brief 根据配置设置数据库连接参数。
     * @param[out] errorMessage 配置失败时接收错误说明。
     * @return 数据库配置成功时返回 @c true。
     */
    [[nodiscard]] bool configureDatabase(QString *errorMessage);
    /**
     * @brief 将数据库结构迁移到当前版本。
     * @param[out] errorMessage 迁移失败时接收错误说明。
     * @return 数据结构可用时返回 @c true。
     */
    [[nodiscard]] bool migrateSchema(QString *errorMessage);
    /**
     * @brief 执行不返回结果集的 SQL 语句。
     * @param statement 待执行的 SQL 语句。
     * @param[out] errorMessage 执行失败时接收错误说明。
     * @return 语句执行成功时返回 @c true。
     */
    [[nodiscard]] bool executeStatement(const QString &statement, QString *errorMessage) const;

    QString m_connectionName;
    QString m_databasePath;
    bool m_initialized = false;
};

#endif // SQLITECHATREPOSITORY_H
