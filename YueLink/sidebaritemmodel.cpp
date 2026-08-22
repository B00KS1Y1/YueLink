#include "sidebaritemmodel.h"

#include <utility>

SidebarItemModel::SidebarItemModel(QObject *parent)
: QAbstractListModel(parent)
{
}

int SidebarItemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant SidebarItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }
    const Item &item = m_items.at(index.row());
    switch (role)
    {
    case ItemIdRole:
        return item.itemId;
    case ItemKindRole:
        return item.itemKind;
    case TitleRole:
        return item.title;
    case InitialRole:
        return item.initial;
    case StatusTextRole:
        return item.statusText;
    case LastMessageRole:
        return item.lastMessage;
    case LastTimeRole:
        return item.lastTime;
    case AvatarColorRole:
        return item.avatarColor;
    case OnlineRole:
        return item.online;
    case UnreadRole:
        return item.unread;
    case PeerIdRole:
        return item.peerId;
    case MemberCountRole:
        return item.memberCount;
    case OnlineCountRole:
        return item.onlineCount;
    case PinnedRole:
        return item.pinned;
    case HiddenRole:
        return item.hidden;
    case SortTimestampRole:
        return item.sortTimestamp;
    default:
        return {};
    }
}

QHash<int, QByteArray> SidebarItemModel::roleNames() const
{
    return {{ItemIdRole, "itemId"},
            {ItemKindRole, "itemKind"},
            {TitleRole, "title"},
            {InitialRole, "initial"},
            {StatusTextRole, "statusText"},
            {LastMessageRole, "lastMessage"},
            {LastTimeRole, "lastTime"},
            {AvatarColorRole, "avatarColor"},
            {OnlineRole, "online"},
            {UnreadRole, "unread"},
            {PeerIdRole, "peerId"},
            {MemberCountRole, "memberCount"},
            {OnlineCountRole, "onlineCount"},
            {PinnedRole, "pinned"},
            {HiddenRole, "hidden"},
            {SortTimestampRole, "sortTimestamp"}};
}

QVariantMap SidebarItemModel::itemInfo(const QString &itemId) const
{
    for (const Item &item : m_items)
    {
        if (item.itemId == itemId)
        {
            return {{QStringLiteral("itemId"), item.itemId},
                    {QStringLiteral("itemKind"), item.itemKind},
                    {QStringLiteral("title"), item.title},
                    {QStringLiteral("initial"), item.initial},
                    {QStringLiteral("statusText"), item.statusText},
                    {QStringLiteral("lastMessage"), item.lastMessage},
                    {QStringLiteral("lastTime"), item.lastTime},
                    {QStringLiteral("avatarColor"), item.avatarColor},
                    {QStringLiteral("online"), item.online},
                    {QStringLiteral("unread"), item.unread},
                    {QStringLiteral("peerId"), item.peerId},
                    {QStringLiteral("memberCount"), item.memberCount},
                    {QStringLiteral("onlineCount"), item.onlineCount},
                    {QStringLiteral("pinned"), item.pinned},
                    {QStringLiteral("hidden"), item.hidden}};
        }
    }
    return {};
}

int SidebarItemModel::onlineCount() const
{
    int count = 0;
    for (const Item &item : m_items)
    {
        count += item.online ? 1 : 0;
    }
    return count;
}

int SidebarItemModel::totalUnreadCount() const
{
    int count = 0;
    for (const Item &item : m_items)
    {
        count += item.unread;
    }
    return count;
}

void SidebarItemModel::setItems(QList<Item> items)
{
    const int previousUnread = totalUnreadCount();
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
    if (previousUnread != totalUnreadCount())
    {
        emit unreadCountChanged();
    }
}
