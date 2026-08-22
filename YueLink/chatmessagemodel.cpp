#include "chatmessagemodel.h"

#include <utility>

namespace
{
constexpr qint64 TimeSeparatorThresholdSeconds = 5 * 60;
}

ChatMessageModel::ChatMessageModel(QObject *parent)
: QAbstractListModel(parent)
{
}

int ChatMessageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_conversations.value(m_currentConversationId).size();
}

QVariant ChatMessageModel::data(const QModelIndex &index, int role) const
{
    const auto conversation = m_conversations.constFind(m_currentConversationId);
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
    case SenderNameRole:
        return message.senderName;
    case SenderInitialRole:
        return message.senderInitial;
    case SenderColorRole:
        return message.senderColor;
    case MessageTextRole:
        return message.messageText;
    case MessageTimeRole:
        return message.messageTime;
    case ShowTimeRole:
        return message.showTime;
    case DeliveryStatusRole:
        return message.deliveryStatus;
    case DeliveredCountRole:
        return message.deliveredCount;
    case TotalRecipientCountRole:
        return message.totalRecipientCount;
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
    case ImageWidthRole:
        return message.imageWidth;
    case ImageHeightRole:
        return message.imageHeight;
    case EmojiPackageIdRole:
        return message.emojiPackageId;
    case EmojiIdRole:
        return message.emojiId;
    case SearchTextRole:
        return QStringLiteral("%1 %2 %3").arg(message.senderName, message.fileName, message.messageText);
    default:
        return {};
    }
}

QHash<int, QByteArray> ChatMessageModel::roleNames() const
{
    return {{MessageIdRole, "messageId"},
            {FromMeRole, "fromMe"},
            {SenderNameRole, "senderName"},
            {SenderInitialRole, "senderInitial"},
            {SenderColorRole, "senderColor"},
            {MessageTextRole, "messageText"},
            {MessageTimeRole, "messageTime"},
            {ShowTimeRole, "showTime"},
            {DeliveryStatusRole, "deliveryStatus"},
            {DeliveredCountRole, "deliveredCount"},
            {TotalRecipientCountRole, "totalRecipientCount"},
            {MessageKindRole, "messageKind"},
            {FileNameRole, "fileName"},
            {FileSizeTextRole, "fileSizeText"},
            {FileProgressRole, "fileProgress"},
            {FilePathRole, "filePath"},
            {FileUrlRole, "fileUrl"},
            {ImageWidthRole, "imageWidth"},
            {ImageHeightRole, "imageHeight"},
            {EmojiPackageIdRole, "emojiPackageId"},
            {EmojiIdRole, "emojiId"},
            {SearchTextRole, "searchText"}};
}

void ChatMessageModel::selectConversation(const QString &conversationId)
{
    if (m_currentConversationId == conversationId)
    {
        return;
    }
    beginResetModel();
    m_currentConversationId = conversationId;
    endResetModel();
}

void ChatMessageModel::setConversation(const QString &conversationId, QList<Message> messages)
{
    QDateTime previousTimestamp;
    for (Message &message : messages)
    {
        message.showTime = message.timestamp.isValid()
                           && (!previousTimestamp.isValid()
                               || previousTimestamp.secsTo(message.timestamp) > TimeSeparatorThresholdSeconds);
        if (message.timestamp.isValid())
        {
            previousTimestamp = message.timestamp;
        }
    }
    if (conversationId == m_currentConversationId)
    {
        beginResetModel();
        m_conversations.insert(conversationId, std::move(messages));
        endResetModel();
        return;
    }
    m_conversations.insert(conversationId, std::move(messages));
}

void ChatMessageModel::removeConversation(const QString &conversationId)
{
    if (conversationId != m_currentConversationId)
    {
        m_conversations.remove(conversationId);
        return;
    }
    beginResetModel();
    m_conversations.remove(conversationId);
    m_currentConversationId.clear();
    endResetModel();
}

void ChatMessageModel::append(const QString &conversationId, Message message)
{
    QList<Message> &messages = m_conversations[conversationId];
    const QDateTime previousTimestamp = messages.isEmpty() ? QDateTime{} : messages.constLast().timestamp;
    message.showTime = message.timestamp.isValid()
                       && (!previousTimestamp.isValid()
                           || previousTimestamp.secsTo(message.timestamp) > TimeSeparatorThresholdSeconds);
    if (conversationId == m_currentConversationId)
    {
        const int row = messages.size();
        beginInsertRows({}, row, row);
        messages.append(std::move(message));
        endInsertRows();
        return;
    }
    messages.append(std::move(message));
}

void ChatMessageModel::updateDelivery(
    const QString &conversationId, const QString &messageId, const QString &status, int deliveredCount, int totalRecipientCount)
{
    QList<Message> &messages = m_conversations[conversationId];
    for (int row = messages.size() - 1; row >= 0; --row)
    {
        Message &message = messages[row];
        if (message.messageId != messageId)
        {
            continue;
        }
        message.deliveryStatus = status;
        if (deliveredCount >= 0)
        {
            message.deliveredCount = deliveredCount;
        }
        if (totalRecipientCount >= 0)
        {
            message.totalRecipientCount = totalRecipientCount;
        }
        if (conversationId == m_currentConversationId)
        {
            emit dataChanged(index(row), index(row), {DeliveryStatusRole, DeliveredCountRole, TotalRecipientCountRole});
        }
        return;
    }
}

void ChatMessageModel::updateFileTransfer(
    const QString &conversationId, const QString &messageId, qreal progress, const QString &status, const QString &filePath)
{
    QList<Message> &messages = m_conversations[conversationId];
    for (int row = messages.size() - 1; row >= 0; --row)
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
        if (conversationId == m_currentConversationId)
        {
            emit dataChanged(index(row), index(row), {FileProgressRole, DeliveryStatusRole, FilePathRole, FileUrlRole});
        }
        return;
    }
}
