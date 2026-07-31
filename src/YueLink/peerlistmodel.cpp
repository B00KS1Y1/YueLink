#include "peerlistmodel.h"

#include <utility>

PeerListModel::PeerListModel(QObject *parent)
: QAbstractListModel(parent)
{
}

int PeerListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_peers.size());
}

QVariant PeerListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_peers.size())
    {
        return {};
    }

    const Item &peer = m_peers.at(index.row());
    switch (role)
    {
    case PeerIdRole:
        return peer.endpoint.peerId;
    case FriendNameRole:
        return peer.endpoint.displayName;
    case InitialRole:
        return peer.initial;
    case StatusTextRole:
        return peer.statusText;
    case LastMessageRole:
        return peer.lastMessage;
    case LastTimeRole:
        return peer.lastTime;
    case AvatarColorRole:
        return peer.avatarColor;
    case OnlineRole:
        return peer.online;
    case UnreadRole:
        return peer.unread;
    case AddressRole:
        return peer.endpoint.address.toString();
    case TcpPortRole:
        return peer.endpoint.tcpPort;
    default:
        return {};
    }
}

QHash<int, QByteArray> PeerListModel::roleNames() const
{
    return {{PeerIdRole, "peerId"},
            {FriendNameRole, "friendName"},
            {InitialRole, "initial"},
            {StatusTextRole, "statusText"},
            {LastMessageRole, "lastMessage"},
            {LastTimeRole, "lastTime"},
            {AvatarColorRole, "avatarColor"},
            {OnlineRole, "online"},
            {UnreadRole, "unread"},
            {AddressRole, "address"},
            {TcpPortRole, "tcpPort"}};
}

int PeerListModel::indexOf(const QString &peerId) const
{
    for (int row = 0; row < m_peers.size(); ++row)
    {
        if (m_peers.at(row).endpoint.peerId == peerId)
        {
            return row;
        }
    }
    return -1;
}

QVariantMap PeerListModel::peerInfo(const QString &peerId) const
{
    const int row = indexOf(peerId);
    if (row < 0)
    {
        return {};
    }
    const Item &peer = m_peers.at(row);
    return {{QStringLiteral("peerId"), peer.endpoint.peerId},
            {QStringLiteral("friendName"), peer.endpoint.displayName},
            {QStringLiteral("initial"), peer.initial},
            {QStringLiteral("statusText"), peer.statusText},
            {QStringLiteral("avatarColor"), peer.avatarColor},
            {QStringLiteral("online"), peer.online},
            {QStringLiteral("address"), peer.endpoint.address.toString()},
            {QStringLiteral("tcpPort"), peer.endpoint.tcpPort}};
}

int PeerListModel::onlineCount() const
{
    int count = 0;
    for (const Item &peer : m_peers)
    {
        count += peer.online ? 1 : 0;
    }
    return count;
}

int PeerListModel::totalUnreadCount() const
{
    int count = 0;
    for (const Item &peer : m_peers)
    {
        count += peer.unread;
    }
    return count;
}

void PeerListModel::setItems(QList<Item> items)
{
    const int previousUnread = totalUnreadCount();
    beginResetModel();
    m_peers = std::move(items);
    endResetModel();
    if (previousUnread != totalUnreadCount())
    {
        emit unreadCountChanged();
    }
}
