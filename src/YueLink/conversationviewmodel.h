/**
 * @file conversationviewmodel.h
 * @brief 声明当前统一会话与消息列表 QML 视图模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef CONVERSATIONVIEWMODEL_H
#define CONVERSATIONVIEWMODEL_H

#include "domain/chattypes.h"
#include "YueLink/chatmessagemodel.h"

#include <QDateTime>
#include <QObject>
#include <QSortFilterProxyModel>

class ChatCoordinator;

class ConversationViewModel final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造当前会话视图模型。
     * @param coordinator 非拥有的聊天协调器。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ConversationViewModel(ChatCoordinator *coordinator,
                                   QObject *parent = nullptr);
    /** @brief 销毁当前会话视图模型。 */
    ~ConversationViewModel() override;
    /**
     * @brief 返回供 QML 使用的消息筛选模型。
     * @return 消息模型指针。
     */
    [[nodiscard]] QAbstractItemModel *model();
    /**
     * @brief 返回消息搜索文本。
     * @return 当前搜索文本。
     */
    [[nodiscard]] QString searchText() const;
    /**
     * @brief 更新消息搜索文本。
     * @param text 新搜索文本。
     */
    void setSearchText(const QString &text);
    /**
     * @brief 返回当前会话标识。
     * @return 当前会话标识；未选择时为空。
     */
    [[nodiscard]] QString currentConversationId() const;
    /**
     * @brief 返回本地显示名称首字符。
     * @return 本地头像首字符。
     */
    [[nodiscard]] QString localInitial() const;
    /**
     * @brief 选择并加载直接会话或群聊。
     * @param conversationId 会话标识。
     * @return 会话存在时返回 @c true。
     */
    [[nodiscard]] bool selectConversation(const QString &conversationId);

signals:
    /** @brief 消息搜索文本发生变化时发出。 */
    void searchTextChanged();
    /** @brief 当前会话发生变化时发出。 */
    void currentConversationIdChanged();

private:
    /**
     * @brief 完整同步指定会话消息。
     * @param conversationId 会话标识。
     */
    void synchronize(const QString &conversationId);
    /**
     * @brief 增量处理新消息。
     * @param message 新消息。
     */
    void handleMessageAdded(const Domain::Message &message);
    /**
     * @brief 处理消息状态变化。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param state 新状态。
     */
    void handleMessageStateChanged(const QString &conversationId,
                                   const QString &messageId,
                                   Domain::DeliveryState state);
    /**
     * @brief 处理逐成员投递统计变化。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param deliveredCount 已发送数量。
     * @param totalCount 接收方总数。
     */
    void handleDeliveryChanged(const QString &conversationId,
                               const QString &messageId,
                               int deliveredCount,
                               int totalCount);
    /**
     * @brief 处理文件传输变化。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param progress 进度。
     * @param state 新状态。
     * @param filePath 本地路径。
     */
    void handleFileTransferChanged(const QString &conversationId,
                                   const QString &messageId,
                                   qreal progress,
                                   Domain::DeliveryState state,
                                   const QString &filePath);
    /**
     * @brief 将领域消息转换为 QML 消息。
     * @param message 领域消息。
     * @return 视图消息。
     */
    [[nodiscard]] ChatMessageModel::Message toViewMessage(
        const Domain::Message &message) const;
    /**
     * @brief 查找消息发送者显示名称。
     * @param message 领域消息。
     * @return 本地资料、联系人或群成员快照中的名称。
     */
    [[nodiscard]] QString senderName(const Domain::Message &message) const;
    /**
     * @brief 格式化文件大小。
     * @param bytes 文件字节数。
     * @param fallback 不可用时的回退文本。
     * @return 本地化文件大小文本。
     */
    [[nodiscard]] static QString displayFileSize(qint64 bytes,
                                                 const QString &fallback);
    /**
     * @brief 格式化消息时间。
     * @param timestamp 消息时间。
     * @return 本地化短时间文本。
     */
    [[nodiscard]] static QString displayTime(const QDateTime &timestamp);
    /**
     * @brief 返回名称首字符。
     * @param name 显示名称。
     * @return 大写首字符或问号。
     */
    [[nodiscard]] static QString initialForName(const QString &name);
    /**
     * @brief 为标识生成稳定头像颜色。
     * @param id 稳定标识。
     * @return 十六进制颜色。
     */
    [[nodiscard]] static QString colorForId(const QString &id);

    ChatCoordinator *m_coordinator = nullptr;
    ChatMessageModel m_model;
    QSortFilterProxyModel m_filterModel;
    QString m_searchText;
    QString m_currentConversationId;
};

#endif // CONVERSATIONVIEWMODEL_H
