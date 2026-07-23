#ifndef PEERLISTMODEL_H
#define PEERLISTMODEL_H

#include "domain/networktypes.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QVariantMap>

class PeerListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        PeerIdRole = Qt::UserRole + 1,
        FriendNameRole,
        InitialRole,
        StatusTextRole,
        LastMessageRole,
        LastTimeRole,
        AvatarColorRole,
        OnlineRole,
        UnreadRole,
        AddressRole,
        TcpPortRole
    };

    struct Item
    {
        Network::PeerEndpoint endpoint;
        QString initial;
        QString statusText;
        QString lastMessage;
        QString lastTime;
        QString avatarColor;
        bool online = false;
        int unread = 0;
    };

    explicit PeerListModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int indexOf(const QString &peerId) const;
    [[nodiscard]] QVariantMap peerInfo(const QString &peerId) const;
    [[nodiscard]] int onlineCount() const;
    [[nodiscard]] int totalUnreadCount() const;
    void setItems(QList<Item> items);

signals:
    void unreadCountChanged();

private:
    QList<Item> m_peers;
};

#endif // PEERLISTMODEL_H
