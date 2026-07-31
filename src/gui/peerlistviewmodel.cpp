#include "peerlistviewmodel.h"

#include "application/chatcoordinator.h"

#include <QCryptographicHash>
#include <QDate>
#include <QStringList>

#include <utility>

PeerListViewModel::PeerListViewModel(ChatCoordinator *coordinator,
                                     QObject *parent)
: QObject(parent)
, m_coordinator(coordinator)
{
    Q_ASSERT(m_coordinator);
    m_filterModel.setSourceModel(&m_model);
    m_filterModel.setFilterRole(PeerListModel::FriendNameRole);
    m_filterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel.setDynamicSortFilter(true);
    connect(&m_model,
            &PeerListModel::unreadCountChanged,
            this,
            &PeerListViewModel::totalUnreadCountChanged);
    connect(m_coordinator,
            &ChatCoordinator::peersChanged,
            this,
            &PeerListViewModel::synchronize);
    connect(m_coordinator,
            &ChatCoordinator::peerDiscovered,
            this,
            &PeerListViewModel::peerDiscovered);
    connect(m_coordinator,
            &ChatCoordinator::peerUpdated,
            this,
            &PeerListViewModel::peerUpdated);
    synchronize();
}

PeerListViewModel::~PeerListViewModel() = default;

QAbstractItemModel *PeerListViewModel::model()
{
    return &m_filterModel;
}

QString PeerListViewModel::searchText() const
{
    return m_searchText;
}

void PeerListViewModel::setSearchText(const QString &text)
{
    if (m_searchText == text)
    {
        return;
    }
    m_searchText = text;
    m_filterModel.setFilterFixedString(text.trimmed());
    emit searchTextChanged();
}

int PeerListViewModel::onlineCount() const
{
    return m_model.onlineCount();
}

int PeerListViewModel::totalUnreadCount() const
{
    return m_model.totalUnreadCount();
}

QVariantMap PeerListViewModel::peerInfo(const QString &peerId) const
{
    return m_model.peerInfo(peerId);
}

void PeerListViewModel::synchronize()
{
    const int previousOnlineCount = onlineCount();
    QList<PeerListModel::Item> items;
    const QList<Domain::Peer> peers = m_coordinator->peers();
    items.reserve(peers.size());
    for (const Domain::Peer &peer : peers)
    {
        PeerListModel::Item item;
        item.endpoint = peer.endpoint;
        item.initial = initialForName(peer.endpoint.displayName);
        item.statusText = peer.online ? tr("在线 · 局域网")
                                      : tr("离线 · 局域网");
        item.lastMessage = peer.lastMessage.isEmpty()
                               ? tr("已通过局域网发现")
                               : peer.lastMessage;
        item.lastTime = displayTime(peer.lastActivity);
        item.avatarColor = colorForId(peer.endpoint.peerId);
        item.online = peer.online;
        item.unread = peer.unreadCount;
        items.append(std::move(item));
    }
    m_model.setItems(std::move(items));
    if (previousOnlineCount != onlineCount())
    {
        emit onlineCountChanged();
    }
}

QString PeerListViewModel::initialForName(const QString &name)
{
    const QString trimmed = name.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("?") : trimmed.left(1).toUpper();
}

QString PeerListViewModel::displayTime(const QDateTime &timestamp)
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

QString PeerListViewModel::colorForId(const QString &peerId)
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
