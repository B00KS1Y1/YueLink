/**
 * @file chatmessagemodel.h
 * @brief 声明供 QML 使用的聊天消息列表模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef CHATMESSAGEMODEL_H
#define CHATMESSAGEMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QUrl>

class ChatMessageModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        MessageIdRole = Qt::UserRole + 1,
        FromMeRole,
        SenderInitialRole,
        SenderColorRole,
        MessageTextRole,
        MessageTimeRole,
        DeliveryStatusRole,
        MessageKindRole,
        FileNameRole,
        FileSizeTextRole,
        FileProgressRole,
        FilePathRole,
        FileUrlRole,
        SearchTextRole
    };

    struct Message
    {
        QString messageId;
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
        bool fromMe = false;
    };

    /**
     * @brief 构造聊天消息列表模型。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ChatMessageModel(QObject *parent = nullptr);

    /**
     * @brief 返回当前会话中的消息数量。
     * @param parent 父模型索引；列表模型应传入无效索引。
     * @return 当前会话的消息数量。
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief 返回指定消息的角色数据。
     * @param index 消息模型索引。
     * @param role 待读取的模型角色。
     * @return 对应角色的数据；索引或角色无效时返回空值。
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    /**
     * @brief 返回 QML 可访问的模型角色名称。
     * @return 角色编号到角色名称的映射。
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief 切换当前显示的会话。
     * @param peerId 新的会话节点标识。
     */
    void selectPeer(const QString &peerId);
    /**
     * @brief 替换指定会话的全部消息。
     * @param peerId 会话对应的节点标识。
     * @param messages 新的消息列表。
     */
    void setConversation(const QString &peerId, QList<Message> messages);
    /**
     * @brief 向指定会话追加消息。
     * @param peerId 会话对应的节点标识。
     * @param message 待追加的消息。
     */
    void append(const QString &peerId, Message message);
    /**
     * @brief 更新指定消息的投递状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param status 新的投递状态。
     */
    void setDeliveryStatus(const QString &peerId, const QString &messageId, const QString &status);
    /**
     * @brief 更新文件传输消息的进度与状态。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 取值范围为 0.0 到 1.0 的传输进度。
     * @param status 新的传输状态。
     * @param filePath 可用时提供本地文件路径。
     */
    void updateFileTransfer(const QString &peerId, const QString &messageId, qreal progress, const QString &status, const QString &filePath = {});

private:
    QString m_currentPeerId;
    QHash<QString, QList<Message>> m_conversations;
};

#endif // CHATMESSAGEMODEL_H
