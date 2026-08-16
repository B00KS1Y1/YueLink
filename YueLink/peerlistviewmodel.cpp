#include "peerlistviewmodel.h"

#include "application/chatcoordinator.h"

#include <QCryptographicHash>

#include <utility>

PeerListViewModel::PeerListViewModel(ChatCoordinator *coordinator, QObject *parent)
: QObject(parent)
, m_coordinator(coordinator)
{
    Q_ASSERT(m_coordinator);
    m_filterModel.setSourceModel(&m_model);
    m_filterModel.setFilterRole(SidebarItemModel::TitleRole);
    m_filterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(m_coordinator, &ChatCoordinator::peersChanged, this, &PeerListViewModel::synchronize);
    connect(m_coordinator, &ChatCoordinator::peerDiscovered, this, &PeerListViewModel::peerDiscovered);
    connect(m_coordinator, &ChatCoordinator::peerUpdated, this, &PeerListViewModel::peerUpdated);
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

QVariantMap PeerListViewModel::peerInfo(const QString &peerId) const
{
    return m_model.itemInfo(Domain::directConversationId(peerId));
}

void PeerListViewModel::synchronize()
{
    const int previousOnline = onlineCount();
    QList<SidebarItemModel::Item> items;
    for (const Domain::Peer &peer : m_coordinator->peers())
    {
        SidebarItemModel::Item item;
        item.itemId = Domain::directConversationId(peer.endpoint.peerId);
        item.itemKind = QStringLiteral("direct");
        item.title = peer.endpoint.displayName;
        item.initial = initialForName(item.title);
        item.statusText = peer.online ? tr("在线 · 局域网") : tr("离线");
        item.lastMessage = item.statusText;
        item.avatarColor = colorForId(peer.endpoint.peerId);
        item.peerId = peer.endpoint.peerId;
        item.memberCount = 2;
        item.onlineCount = peer.online ? 1 : 0;
        item.online = peer.online;
        items.append(std::move(item));
    }
    m_model.setItems(std::move(items));
    if (previousOnline != onlineCount())
    {
        emit onlineCountChanged();
    }
}

QString PeerListViewModel::initialForName(const QString &name)
{
    const QString normalized = name.trimmed();
    return normalized.isEmpty() ? QStringLiteral("?") : normalized.left(1).toUpper();
}

QString PeerListViewModel::colorForId(const QString &id)
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
