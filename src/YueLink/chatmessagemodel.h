/**
 * @file chatmessagemodel.h
 * @brief 声明统一会话消息 QML 列表模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef CHATMESSAGEMODEL_H
#define CHATMESSAGEMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QString>
#include <QUrl>

class ChatMessageModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        MessageIdRole = Qt::UserRole + 1,
        FromMeRole,
        SenderNameRole,
        SenderInitialRole,
        SenderColorRole,
        MessageTextRole,
        MessageTimeRole,
        DeliveryStatusRole,
        DeliveredCountRole,
        TotalRecipientCountRole,
        MessageKindRole,
        FileNameRole,
        FileSizeTextRole,
        FileProgressRole,
        FilePathRole,
        FileUrlRole,
        ImageWidthRole,
        ImageHeightRole,
        EmojiPackageIdRole,
        EmojiIdRole,
        SearchTextRole
    };

    struct Message
    {
        QString messageId;
        QString senderName;
        QString senderInitial;
        QString senderColor;
        QString messageText;
        QString messageTime;
        QString deliveryStatus;
        QString messageKind = QStringLiteral("text");
        QString fileName;
        QString fileSizeText;
        QString filePath;
        QUrl fileUrl;
        qreal fileProgress = 0.0;
        int imageWidth = 0;
        int imageHeight = 0;
        QString emojiPackageId;
        QString emojiId;
        int deliveredCount = 0;
        int totalRecipientCount = 0;
        bool fromMe = false;
    };

    /**
     * @brief 构造消息模型。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ChatMessageModel(QObject *parent = nullptr);
    /**
     * @brief 返回当前会话消息数量。
     * @param parent 父索引；列表模型应传入无效索引。
     * @return 消息数量。
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief 返回消息角色数据。
     * @param index 消息索引。
     * @param role 角色编号。
     * @return 角色数据；无效时为空。
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    /**
     * @brief 返回 QML 角色名称。
     * @return 角色编号到名称的映射。
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    /**
     * @brief 切换当前会话。
     * @param conversationId 新会话标识。
     */
    void selectConversation(const QString &conversationId);
    /**
     * @brief 替换指定会话全部消息。
     * @param conversationId 会话标识。
     * @param messages 新消息列表。
     */
    void setConversation(const QString &conversationId, QList<Message> messages);
    /**
     * @brief 向指定会话追加消息。
     * @param conversationId 会话标识。
     * @param message 新消息。
     */
    void append(const QString &conversationId, Message message);
    /**
     * @brief 更新消息状态与群投递统计。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param status 状态名称。
     * @param deliveredCount 已发送数量；负值表示保持原值。
     * @param totalRecipientCount 接收方总数；负值表示保持原值。
     */
    void updateDelivery(const QString &conversationId,
                        const QString &messageId,
                        const QString &status,
                        int deliveredCount = -1,
                        int totalRecipientCount = -1);
    /**
     * @brief 更新文件消息传输信息。
     * @param conversationId 会话标识。
     * @param messageId 消息标识。
     * @param progress 传输进度。
     * @param status 新状态。
     * @param filePath 可用时提供本地路径。
     */
    void updateFileTransfer(const QString &conversationId,
                            const QString &messageId,
                            qreal progress,
                            const QString &status,
                            const QString &filePath = {});

private:
    QHash<QString, QList<Message>> m_conversations;
    QString m_currentConversationId;
};

#endif // CHATMESSAGEMODEL_H
