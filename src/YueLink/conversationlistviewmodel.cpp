#include "conversationlistviewmodel.h"

#include "application/chatcoordinator.h"

#include <QCryptographicHash>
#include <QLocale>

#include <utility>

ConversationListViewModel::ConversationListViewModel(ChatCoordinator *coordinator, QObject *parent)
: QObject(parent)
, m_coordinator(coordinator)
{
    Q_ASSERT(m_coordinator);
    m_filterModel.setSourceModel(&m_model);
    m_filterModel.setFilterRole(SidebarItemModel::TitleRole);
    m_filterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel.setSortRole(SidebarItemModel::SortTimestampRole);
    m_filterModel.sort(0, Qt::DescendingOrder);
    connect(m_coordinator, &ChatCoordinator::conversationsChanged, this, &ConversationListViewModel::synchronize);
    connect(m_coordinator, &ChatCoordinator::peersChanged, this, &ConversationListViewModel::synchronize);
    connect(&m_model, &SidebarItemModel::unreadCountChanged, this, &ConversationListViewModel::totalUnreadCountChanged);
    synchronize();
}

ConversationListViewModel::~ConversationListViewModel() = default;
QAbstractItemModel *ConversationListViewModel::model()
{
    return &m_filterModel;
}
QString ConversationListViewModel::searchText() const
{
    return m_searchText;
}

void ConversationListViewModel::setSearchText(const QString &text)
{
    if (m_searchText == text)
    {
        return;
    }
    m_searchText = text;
    m_filterModel.setFilterFixedString(text.trimmed());
    emit searchTextChanged();
}

QVariantMap ConversationListViewModel::conversationInfo(const QString &conversationId) const
{
    return m_model.itemInfo(conversationId);
}

int ConversationListViewModel::totalUnreadCount() const
{
    return m_model.totalUnreadCount();
}

void ConversationListViewModel::synchronize()
{
    QList<SidebarItemModel::Item> items;
    for (const Domain::Conversation &conversation : m_coordinator->conversations())
    {
        SidebarItemModel::Item item;
        item.itemId = conversation.conversationId;
        item.itemKind = Domain::conversationKindName(conversation.kind);
        item.title = conversation.title;
        item.initial = initialForName(item.title);
        item.lastMessage = conversation.lastMessage.isEmpty() ? conversation.kind == Domain::ConversationKind::Group ? tr("群聊已创建") : tr("开始聊天")
                                                              : conversation.lastMessage;
        item.lastTime = displayTime(conversation.lastActivity);
        item.avatarColor = colorForId(conversation.conversationId);
        item.peerId = conversation.peerId;
        item.unread = conversation.unreadCount;
        item.memberCount = conversation.memberCount;
        item.sortTimestamp = conversation.lastActivity.toMSecsSinceEpoch();
        if (conversation.kind == Domain::ConversationKind::Direct)
        {
            Domain::Peer peer;
            item.online = m_coordinator->peer(conversation.peerId, &peer) && peer.online;
            item.onlineCount = item.online ? 1 : 0;
            item.statusText = item.online ? tr("在线 · 局域网") : tr("离线");
        }
        else
        {
            int onlineMembers = m_coordinator->running() ? 1 : 0;
            const QList<Domain::GroupMember> members = m_coordinator->groupMembers(conversation.conversationId);
            for (const Domain::GroupMember &member : members)
            {
                if (member.peerId == m_coordinator->localIdentity().deviceId)
                {
                    continue;
                }
                Domain::Peer peer;
                onlineMembers += m_coordinator->peer(member.peerId, &peer) && peer.online ? 1 : 0;
            }
            item.memberCount = members.size();
            item.onlineCount = onlineMembers;
            item.online = onlineMembers > 1;
            item.statusText = tr("%1 位成员 · %2 人在线").arg(item.memberCount).arg(item.onlineCount);
        }
        items.append(std::move(item));
    }
    m_model.setItems(std::move(items));
    emit conversationsChanged();
}

QString ConversationListViewModel::displayTime(const QDateTime &timestamp)
{
    if (!timestamp.isValid())
    {
        return {};
    }
    const QDateTime local = timestamp.toLocalTime();
    const QDate currentDate = QDate::currentDate();
    return local.date() == currentDate ? QLocale().toString(local.time(), QLocale::ShortFormat) : QLocale().toString(local.date(), QLocale::ShortFormat);
}

QString ConversationListViewModel::initialForName(const QString &name)
{
    const QString normalized = name.trimmed();
    return normalized.isEmpty() ? QStringLiteral("?") : normalized.left(1).toUpper();
}

QString ConversationListViewModel::colorForId(const QString &id)
{
    static const QStringList colors{QStringLiteral("#4F7CFF"),
                                    QStringLiteral("#7C6EE6"),
                                    QStringLiteral("#2CA58D"),
                                    QStringLiteral("#D97757"),
                                    QStringLiteral("#C2548A"),
                                    QStringLiteral("#65758B")};
    const QByteArray digest = QCryptographicHash::hash(id.toUtf8(), QCryptographicHash::Sha256);
    return colors.at(static_cast<unsigned char>(digest.at(0)) % colors.size());
}
