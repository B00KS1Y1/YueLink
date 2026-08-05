#include "conversationviewmodel.h"

#include "application/chatcoordinator.h"

#include <QCryptographicHash>
#include <QLocale>
#include <QUrl>

#include <utility>

ConversationViewModel::ConversationViewModel(ChatCoordinator *coordinator,
                                             QObject *parent)
: QObject(parent)
, m_coordinator(coordinator)
{
    Q_ASSERT(m_coordinator);
    m_filterModel.setSourceModel(&m_model);
    m_filterModel.setFilterRole(ChatMessageModel::SearchTextRole);
    m_filterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(m_coordinator, &ChatCoordinator::messageAdded,
            this, &ConversationViewModel::handleMessageAdded);
    connect(m_coordinator, &ChatCoordinator::messageStateChanged,
            this, &ConversationViewModel::handleMessageStateChanged);
    connect(m_coordinator, &ChatCoordinator::deliveryChanged,
            this, &ConversationViewModel::handleDeliveryChanged);
    connect(m_coordinator, &ChatCoordinator::fileTransferChanged,
            this, &ConversationViewModel::handleFileTransferChanged);
}

ConversationViewModel::~ConversationViewModel() = default;
QAbstractItemModel *ConversationViewModel::model() { return &m_filterModel; }
QString ConversationViewModel::searchText() const { return m_searchText; }

void ConversationViewModel::setSearchText(const QString &text)
{
    if (m_searchText == text)
        return;
    m_searchText = text;
    m_filterModel.setFilterFixedString(text.trimmed());
    emit searchTextChanged();
}

QString ConversationViewModel::currentConversationId() const
{
    return m_currentConversationId;
}

QString ConversationViewModel::localInitial() const
{
    return initialForName(m_coordinator->localIdentity().displayName);
}

bool ConversationViewModel::selectConversation(const QString &conversationId)
{
    Domain::Conversation conversation;
    if (!m_coordinator->conversation(conversationId, &conversation))
        return false;
    const bool changed = m_currentConversationId != conversationId;
    m_currentConversationId = conversationId;
    m_searchText.clear();
    m_filterModel.setFilterFixedString({});
    emit searchTextChanged();
    synchronize(conversationId);
    static_cast<void>(m_coordinator->markConversationRead(conversationId));
    if (changed)
        emit currentConversationIdChanged();
    return true;
}

void ConversationViewModel::synchronize(const QString &conversationId)
{
    QList<ChatMessageModel::Message> values;
    for (const Domain::Message &message : m_coordinator->messages(conversationId))
        values.append(toViewMessage(message));
    m_model.setConversation(conversationId, std::move(values));
    m_model.selectConversation(conversationId);
}

void ConversationViewModel::handleMessageAdded(const Domain::Message &message)
{
    m_model.append(message.conversationId, toViewMessage(message));
}

void ConversationViewModel::handleMessageStateChanged(
    const QString &conversationId,
    const QString &messageId,
    Domain::DeliveryState state)
{
    m_model.updateDelivery(conversationId,
                           messageId,
                           Domain::deliveryStateName(state));
}

void ConversationViewModel::handleDeliveryChanged(const QString &conversationId,
                                                   const QString &messageId,
                                                   int deliveredCount,
                                                   int totalCount)
{
    Domain::DeliveryState state = deliveredCount == totalCount && totalCount > 0
                                      ? Domain::DeliveryState::Sent
                                      : deliveredCount > 0
                                            ? Domain::DeliveryState::Partial
                                            : Domain::DeliveryState::Sending;
    m_model.updateDelivery(conversationId,
                           messageId,
                           Domain::deliveryStateName(state),
                           deliveredCount,
                           totalCount);
}

void ConversationViewModel::handleFileTransferChanged(
    const QString &conversationId,
    const QString &messageId,
    qreal progress,
    Domain::DeliveryState state,
    const QString &filePath)
{
    m_model.updateFileTransfer(conversationId,
                               messageId,
                               progress,
                               Domain::deliveryStateName(state),
                               filePath);
}

ChatMessageModel::Message ConversationViewModel::toViewMessage(
    const Domain::Message &message) const
{
    ChatMessageModel::Message view;
    const QString name = senderName(message);
    view.messageId = message.messageId;
    view.senderName = name;
    view.senderInitial = initialForName(name);
    view.senderColor = colorForId(message.senderId);
    view.messageText = message.text;
    view.messageTime = displayTime(message.timestamp);
    view.deliveryStatus = Domain::deliveryStateName(message.deliveryState);
    view.messageKind = Domain::messageKindName(message.kind);
    view.fileName = message.fileName;
    view.fileSizeText = displayFileSize(message.fileSize,
                                        message.legacyFileSizeText);
    view.fileProgress = message.fileProgress;
    view.filePath = message.filePath;
    view.fileUrl = message.filePath.isEmpty() ? QUrl{}
                                              : QUrl::fromLocalFile(message.filePath);
    view.fromMe = message.senderId == m_coordinator->localIdentity().deviceId;
    m_coordinator->deliveryCounts(message.messageId,
                                  &view.deliveredCount,
                                  &view.totalRecipientCount);
    return view;
}

QString ConversationViewModel::senderName(const Domain::Message &message) const
{
    if (message.senderId == m_coordinator->localIdentity().deviceId)
        return m_coordinator->localIdentity().displayName;
    Domain::Peer peer;
    if (m_coordinator->peer(message.senderId, &peer))
        return peer.endpoint.displayName;
    for (const Domain::GroupMember &member :
         m_coordinator->groupMembers(message.conversationId))
    {
        if (member.peerId == message.senderId)
            return member.displayName;
    }
    return tr("未知成员");
}

QString ConversationViewModel::displayFileSize(qint64 bytes,
                                               const QString &fallback)
{
    if (bytes <= 0)
        return fallback;
    constexpr qreal KiB = 1024.0;
    constexpr qreal MiB = KiB * 1024.0;
    constexpr qreal GiB = MiB * 1024.0;
    if (bytes >= GiB)
        return tr("%1 GB").arg(QLocale().toString(bytes / GiB, 'f', 2));
    if (bytes >= MiB)
        return tr("%1 MB").arg(QLocale().toString(bytes / MiB, 'f', 1));
    if (bytes >= KiB)
        return tr("%1 KB").arg(QLocale().toString(bytes / KiB, 'f', 1));
    return tr("%1 字节").arg(QLocale().toString(bytes));
}

QString ConversationViewModel::displayTime(const QDateTime &timestamp)
{
    return timestamp.isValid()
               ? QLocale().toString(timestamp.toLocalTime().time(), QLocale::ShortFormat)
               : QString{};
}

QString ConversationViewModel::initialForName(const QString &name)
{
    const QString normalized = name.trimmed();
    return normalized.isEmpty() ? QStringLiteral("?") : normalized.left(1).toUpper();
}

QString ConversationViewModel::colorForId(const QString &id)
{
    static const QStringList colors{QStringLiteral("#4F7CFF"),
                                    QStringLiteral("#7C6EE6"),
                                    QStringLiteral("#2CA58D"),
                                    QStringLiteral("#D97757"),
                                    QStringLiteral("#C2548A"),
                                    QStringLiteral("#65758B")};
    const QByteArray digest = QCryptographicHash::hash(id.toUtf8(),
                                                       QCryptographicHash::Sha256);
    return colors.at(static_cast<unsigned char>(digest.at(0)) % colors.size());
}
