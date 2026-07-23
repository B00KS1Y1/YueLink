#ifndef CHATMESSAGEMODEL_H
#define CHATMESSAGEMODEL_H

#include <QAbstractListModel>
#include <QHash>

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
        FilePathRole
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
        qreal fileProgress = 0.0;
        bool fromMe = false;
    };

    explicit ChatMessageModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void selectPeer(const QString &peerId);
    void setConversation(const QString &peerId, QList<Message> messages);
    void append(const QString &peerId, Message message);
    void setDeliveryStatus(const QString &peerId, const QString &messageId, const QString &status);
    void updateFileTransfer(const QString &peerId, const QString &messageId, qreal progress, const QString &status, const QString &filePath = {});

private:
    QString m_currentPeerId;
    QHash<QString, QList<Message>> m_conversations;
};

#endif // CHATMESSAGEMODEL_H
