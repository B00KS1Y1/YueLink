#include "chatmessagemodel.h"

#include <utility>

ChatMessageModel::ChatMessageModel(QObject *parent)
: QAbstractListModel(parent)
{
}

int ChatMessageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_conversations.value(m_currentPeerId).size());
}

QVariant ChatMessageModel::data(const QModelIndex &index, int role) const
{
    const auto conversation = m_conversations.constFind(m_currentPeerId);
    if (conversation == m_conversations.cend() || !index.isValid() || index.row() < 0 || index.row() >= conversation->size())
    {
        return {};
    }

    const Message &message = conversation->at(index.row());
    switch (role)
    {
    case MessageIdRole:
        return message.messageId;
    case FromMeRole:
        return message.fromMe;
    case SenderInitialRole:
        return message.senderInitial;
    case SenderColorRole:
        return message.senderColor;
    case MessageTextRole:
        return message.messageText;
    case MessageTimeRole:
        return message.messageTime;
    case DeliveryStatusRole:
        return message.deliveryStatus;
    case MessageKindRole:
        return message.messageKind;
    case FileNameRole:
        return message.fileName;
    case FileSizeTextRole:
        return message.fileSizeText;
    case FileProgressRole:
        return message.fileProgress;
    case FilePathRole:
        return message.filePath;
    case FileUrlRole:
        return message.fileUrl;
    case SearchTextRole:
        return message.messageKind == QStringLiteral("file") ? QStringLiteral("%1 %2").arg(message.fileName, message.messageText) : message.messageText;
    default:
        return {};
    }
}

QHash<int, QByteArray> ChatMessageModel::roleNames() const
{
    return {{MessageIdRole, "messageId"},
            {FromMeRole, "fromMe"},
            {SenderInitialRole, "senderInitial"},
            {SenderColorRole, "senderColor"},
            {MessageTextRole, "messageText"},
            {MessageTimeRole, "messageTime"},
            {DeliveryStatusRole, "deliveryStatus"},
            {MessageKindRole, "messageKind"},
            {FileNameRole, "fileName"},
            {FileSizeTextRole, "fileSizeText"},
            {FileProgressRole, "fileProgress"},
            {FilePathRole, "filePath"},
            {FileUrlRole, "fileUrl"},
            {SearchTextRole, "searchText"}};
}

void ChatMessageModel::selectPeer(const QString &peerId)
{
    if (m_currentPeerId == peerId)
    {
        return;
    }
    beginResetModel();
    m_currentPeerId = peerId;
    endResetModel();
}

void ChatMessageModel::setConversation(const QString &peerId, QList<Message> messages)
{
    if (peerId == m_currentPeerId)
    {
        beginResetModel();
        m_conversations.insert(peerId, std::move(messages));
        endResetModel();
        return;
    }
    m_conversations.insert(peerId, std::move(messages));
}

void ChatMessageModel::append(const QString &peerId, Message message)
{
    QList<Message> &messages = m_conversations[peerId];
    if (peerId == m_currentPeerId)
    {
        const int row = static_cast<int>(messages.size());
        beginInsertRows({}, row, row);
        messages.append(std::move(message));
        endInsertRows();
        return;
    }
    messages.append(std::move(message));
}

void ChatMessageModel::setDeliveryStatus(const QString &peerId, const QString &messageId, const QString &status)
{
    QList<Message> &messages = m_conversations[peerId];
    for (int row = static_cast<int>(messages.size()) - 1; row >= 0; --row)
    {
        Message &message = messages[row];
        if (message.messageId != messageId)
        {
            continue;
        }
        if (message.deliveryStatus == status)
        {
            return;
        }
        message.deliveryStatus = status;
        if (peerId == m_currentPeerId)
        {
            const QModelIndex modelIndex = index(row);
            emit dataChanged(modelIndex, modelIndex, {DeliveryStatusRole});
        }
        return;
    }
}

void ChatMessageModel::updateFileTransfer(const QString &peerId, const QString &messageId, qreal progress, const QString &status, const QString &filePath)
{
    QList<Message> &messages = m_conversations[peerId];
    for (int row = static_cast<int>(messages.size()) - 1; row >= 0; --row)
    {
        Message &message = messages[row];
        if (message.messageId != messageId)
        {
            continue;
        }

        message.fileProgress = qBound(0.0, progress, 1.0);
        message.deliveryStatus = status;
        if (!filePath.isEmpty())
        {
            message.filePath = filePath;
            message.fileUrl = QUrl::fromLocalFile(filePath);
        }
        if (peerId == m_currentPeerId)
        {
            const QModelIndex modelIndex = index(row);
            emit dataChanged(modelIndex, modelIndex, {FileProgressRole, DeliveryStatusRole, FilePathRole, FileUrlRole});
        }
        return;
    }
}
