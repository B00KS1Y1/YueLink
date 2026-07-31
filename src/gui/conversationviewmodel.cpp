#include "conversationviewmodel.h"

#include "application/chatcoordinator.h"

#include <QCryptographicHash>
#include <QDate>
#include <QStringList>
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
    m_filterModel.setDynamicSortFilter(true);
    connect(m_coordinator,
            &ChatCoordinator::messageAdded,
            this,
            &ConversationViewModel::handleMessageAdded);
    connect(m_coordinator,
            &ChatCoordinator::messageStateChanged,
            this,
            &ConversationViewModel::handleMessageStateChanged);
    connect(m_coordinator,
            &ChatCoordinator::fileTransferChanged,
            this,
            &ConversationViewModel::handleFileTransferChanged);
}

ConversationViewModel::~ConversationViewModel() = default;

QAbstractItemModel *ConversationViewModel::model()
{
    return &m_filterModel;
}

QString ConversationViewModel::searchText() const
{
    return m_searchText;
}

void ConversationViewModel::setSearchText(const QString &text)
{
    if (m_searchText == text)
    {
        return;
    }
    m_searchText = text;
    m_filterModel.setFilterFixedString(text.trimmed());
    emit searchTextChanged();
}

QString ConversationViewModel::currentPeerId() const
{
    return m_currentPeerId;
}

QString ConversationViewModel::localInitial() const
{
    return initialForName(m_coordinator->localIdentity().displayName);
}

bool ConversationViewModel::selectPeer(const QString &peerId)
{
    Domain::Peer peer;
    if (!m_coordinator->peer(peerId, &peer))
    {
        return false;
    }
    const bool changed = m_currentPeerId != peerId;
    m_currentPeerId = peerId;
    if (changed)
    {
        setSearchText({});
    }
    synchronize(peerId);
    static_cast<void>(m_coordinator->markConversationRead(peerId));
    if (changed)
    {
        emit currentPeerIdChanged();
    }
    return true;
}

void ConversationViewModel::synchronize(const QString &peerId)
{
    QList<ChatMessageModel::Message> viewMessages;
    const QList<Domain::Message> messages = m_coordinator->messages(peerId);
    viewMessages.reserve(messages.size());
    for (const Domain::Message &message : messages)
    {
        viewMessages.append(toViewMessage(message));
    }
    m_model.setConversation(peerId, std::move(viewMessages));
    m_model.selectPeer(peerId);
}

void ConversationViewModel::handleMessageAdded(const Domain::Message &message)
{
    m_model.append(message.peerId, toViewMessage(message));
}

void ConversationViewModel::handleMessageStateChanged(const QString &peerId,
                                                      const QString &messageId,
                                                      Domain::DeliveryState state)
{
    m_model.setDeliveryStatus(peerId,
                              messageId,
                              Domain::deliveryStateName(state));
}

void ConversationViewModel::handleFileTransferChanged(const QString &peerId,
                                                      const QString &messageId,
                                                      qreal progress,
                                                      Domain::DeliveryState state,
                                                      const QString &filePath)
{
    m_model.updateFileTransfer(peerId,
                               messageId,
                               progress,
                               Domain::deliveryStateName(state),
                               filePath);
}

ChatMessageModel::Message ConversationViewModel::toViewMessage(const Domain::Message &message) const
{
    Domain::Peer peer;
    static_cast<void>(m_coordinator->peer(message.peerId, &peer));
    ChatMessageModel::Message view;
    view.messageId = message.messageId;
    view.senderInitial = message.fromMe
                             ? localInitial()
                             : initialForName(peer.endpoint.displayName);
    view.senderColor = message.fromMe ? QStringLiteral("#4F7CFF")
                                      : colorForId(message.peerId);
    view.messageText = message.text;
    view.messageTime = displayTime(message.timestamp);
    view.deliveryStatus = Domain::deliveryStateName(message.deliveryState);
    view.messageKind = Domain::messageKindName(message.kind);
    view.fileName = message.fileName;
    view.fileSizeText = displayFileSize(message.fileSize,
                                        message.legacyFileSizeText);
    view.fileProgress = message.fileProgress;
    view.filePath = message.filePath;
    view.fileUrl = QUrl::fromLocalFile(message.filePath);
    view.fromMe = message.fromMe;
    return view;
}

QString ConversationViewModel::displayFileSize(qint64 bytes,
                                               const QString &fallback)
{
    if (bytes <= 0)
    {
        return fallback;
    }
    constexpr qreal Kilobyte = 1024.0;
    constexpr qreal Megabyte = Kilobyte * 1024.0;
    constexpr qreal Gigabyte = Megabyte * 1024.0;
    if (bytes < 1024)
    {
        return tr("%1 B").arg(bytes);
    }
    if (bytes < static_cast<qint64>(Megabyte))
    {
        return tr("%1 KB").arg(QString::number(bytes / Kilobyte, 'f', 1));
    }
    if (bytes < static_cast<qint64>(Gigabyte))
    {
        return tr("%1 MB").arg(QString::number(bytes / Megabyte, 'f', 1));
    }
    return tr("%1 GB").arg(QString::number(bytes / Gigabyte, 'f', 1));
}

QString ConversationViewModel::displayTime(const QDateTime &timestamp)
{
    if (!timestamp.isValid())
    {
        return {};
    }
    const QDateTime localTimestamp = timestamp.toLocalTime();
    const QDate currentDate = QDate::currentDate();
    if (localTimestamp.date() == currentDate)
    {
        return localTimestamp.toString(QStringLiteral("HH:mm"));
    }
    if (localTimestamp.date() == currentDate.addDays(-1))
    {
        return tr("昨天 %1").arg(localTimestamp.toString(QStringLiteral("HH:mm")));
    }
    return localTimestamp.toString(QStringLiteral("MM-dd HH:mm"));
}

QString ConversationViewModel::initialForName(const QString &name)
{
    const QString trimmed = name.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("?") : trimmed.left(1).toUpper();
}

QString ConversationViewModel::colorForId(const QString &peerId)
{
    static const QStringList colors = {QStringLiteral("#7C6EE6"),
                                       QStringLiteral("#36A18B"),
                                       QStringLiteral("#D86B87"),
                                       QStringLiteral("#4F8EC9"),
                                       QStringLiteral("#C17A3A"),
                                       QStringLiteral("#6C8D3F")};
    const QByteArray digest = QCryptographicHash::hash(peerId.toUtf8(),
                                                       QCryptographicHash::Sha256);
    if (digest.isEmpty())
    {
        return colors.first();
    }
    const qsizetype index = static_cast<unsigned char>(digest.at(0)) % colors.size();
    return colors.at(index);
}
