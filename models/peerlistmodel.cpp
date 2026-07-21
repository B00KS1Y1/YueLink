#include "peerlistmodel.h"

#include <QCryptographicHash>
#include <QStringList>

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

    const PeerItem &peer = m_peers.at(index.row());
    switch (role)
    {
    case PeerIdRole:
        return peer.endpoint.peerId;
    case FriendNameRole:
        return peer.endpoint.displayName;
    case InitialRole:
        return peer.initial;
    case StatusTextRole:
        return peer.online ? tr("在线 · 局域网") : tr("离线 · 局域网");
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
    return peerInfoAt(indexOf(peerId));
}

Network::PeerEndpoint PeerListModel::endpoint(const QString &peerId) const
{
    const int row = indexOf(peerId);
    return row < 0 ? Network::PeerEndpoint() : m_peers.at(row).endpoint;
}

bool PeerListModel::isOnline(const QString &peerId) const
{
    const int row = indexOf(peerId);
    return row >= 0 && m_peers.at(row).online;
}

int PeerListModel::onlineCount() const
{
    int count = 0;
    for (const PeerItem &peer : m_peers)
    {
        count += peer.online ? 1 : 0;
    }
    return count;
}

int PeerListModel::totalUnreadCount() const
{
    int count = 0;
    for (const PeerItem &peer : m_peers)
    {
        count += peer.unread;
    }
    return count;
}

bool PeerListModel::restore(const Network::PeerEndpoint &endpoint, const QString &lastMessage, const QString &lastTime, int unreadCount)
{
    if (endpoint.peerId.isEmpty() || endpoint.displayName.isEmpty() || indexOf(endpoint.peerId) >= 0)
    {
        return false;
    }

    PeerItem peer;
    peer.endpoint = endpoint;
    peer.initial = initialForName(endpoint.displayName);
    peer.avatarColor = colorForId(endpoint.peerId);
    peer.lastMessage = lastMessage.isEmpty() ? tr("已通过局域网发现") : lastMessage;
    peer.lastTime = lastTime;
    peer.online = false;
    peer.unread = qMax(0, unreadCount);

    const int row = static_cast<int>(m_peers.size());
    beginInsertRows({}, row, row);
    m_peers.append(std::move(peer));
    endInsertRows();
    if (unreadCount > 0)
    {
        emit unreadCountChanged();
    }
    return true;
}

bool PeerListModel::upsert(const Network::PeerEndpoint &endpoint, bool *inserted)
{
    const int existingRow = indexOf(endpoint.peerId);
    if (existingRow < 0)
    {
        if (inserted)
        {
            *inserted = true;
        }

        PeerItem peer;
        peer.endpoint = endpoint;
        peer.initial = initialForName(endpoint.displayName);
        peer.avatarColor = colorForId(endpoint.peerId);
        peer.lastMessage = tr("已通过局域网发现");
        peer.lastTime = tr("刚刚");
        peer.online = true;

        const int row = static_cast<int>(m_peers.size());
        beginInsertRows({}, row, row);
        m_peers.append(std::move(peer));
        endInsertRows();
        return true;
    }

    if (inserted)
    {
        *inserted = false;
    }

    PeerItem &peer = m_peers[existingRow];
    const bool changed = peer.endpoint.displayName != endpoint.displayName || peer.endpoint.address != endpoint.address ||
                         peer.endpoint.tcpPort != endpoint.tcpPort || !peer.online;
    peer.endpoint = endpoint;
    peer.initial = initialForName(endpoint.displayName);
    peer.online = true;
    if (changed)
    {
        const QModelIndex modelIndex = index(existingRow);
        emit dataChanged(modelIndex, modelIndex, {FriendNameRole, InitialRole, StatusTextRole, OnlineRole, AddressRole, TcpPortRole});
    }
    return changed;
}

bool PeerListModel::setOffline(const QString &peerId)
{
    const int row = indexOf(peerId);
    if (row < 0 || !m_peers.at(row).online)
    {
        return false;
    }

    m_peers[row].online = false;
    const QModelIndex modelIndex = index(row);
    emit dataChanged(modelIndex, modelIndex, {StatusTextRole, OnlineRole});
    return true;
}

void PeerListModel::updateConversation(const QString &peerId, const QString &message, const QString &time, bool incrementUnread)
{
    const int row = indexOf(peerId);
    if (row < 0)
    {
        return;
    }

    PeerItem &peer = m_peers[row];
    peer.lastMessage = message;
    peer.lastTime = time;
    if (incrementUnread)
    {
        ++peer.unread;
    }
    int updatedRow = row;
    if (row > 0)
    {
        beginMoveRows({}, row, row, {}, 0);
        PeerItem updatedPeer = m_peers.takeAt(row);
        m_peers.prepend(std::move(updatedPeer));
        endMoveRows();
        updatedRow = 0;
    }
    const QModelIndex modelIndex = index(updatedRow);
    emit dataChanged(modelIndex, modelIndex, {LastMessageRole, LastTimeRole, UnreadRole});
    if (incrementUnread)
    {
        emit unreadCountChanged();
    }
}

void PeerListModel::clearUnread(const QString &peerId)
{
    const int row = indexOf(peerId);
    if (row < 0 || m_peers.at(row).unread == 0)
    {
        return;
    }

    m_peers[row].unread = 0;
    const QModelIndex modelIndex = index(row);
    emit dataChanged(modelIndex, modelIndex, {UnreadRole});
    emit unreadCountChanged();
}

QVariantMap PeerListModel::peerInfoAt(int row) const
{
    if (row < 0 || row >= m_peers.size())
    {
        return {};
    }

    const PeerItem &peer = m_peers.at(row);
    return {{QStringLiteral("peerId"), peer.endpoint.peerId},
            {QStringLiteral("friendName"), peer.endpoint.displayName},
            {QStringLiteral("initial"), peer.initial},
            {QStringLiteral("statusText"), peer.online ? tr("在线 · 局域网") : tr("离线 · 局域网")},
            {QStringLiteral("avatarColor"), peer.avatarColor},
            {QStringLiteral("online"), peer.online},
            {QStringLiteral("address"), peer.endpoint.address.toString()},
            {QStringLiteral("tcpPort"), peer.endpoint.tcpPort}};
}

QString PeerListModel::initialForName(const QString &name)
{
    const QString trimmed = name.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("?") : trimmed.left(1).toUpper();
}

QString PeerListModel::colorForId(const QString &peerId)
{
    static const QStringList colors = {QStringLiteral("#7C6EE6"),
                                       QStringLiteral("#36A18B"),
                                       QStringLiteral("#D86B87"),
                                       QStringLiteral("#4F8EC9"),
                                       QStringLiteral("#C17A3A"),
                                       QStringLiteral("#6C8D3F")};
    const QByteArray digest = QCryptographicHash::hash(peerId.toUtf8(), QCryptographicHash::Sha256);
    const qsizetype colorIndex = static_cast<unsigned char>(digest.at(0)) % colors.size();
    return colors.at(colorIndex);
}
