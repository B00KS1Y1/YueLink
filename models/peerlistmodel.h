#ifndef PEERLISTMODEL_H
#define PEERLISTMODEL_H

#include "domain/networktypes.h"

#include <QAbstractListModel>
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

    explicit PeerListModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int indexOf(const QString &peerId) const;
    [[nodiscard]] QVariantMap peerInfo(const QString &peerId) const;
    [[nodiscard]] Network::PeerEndpoint endpoint(const QString &peerId) const;
    [[nodiscard]] bool isOnline(const QString &peerId) const;
    [[nodiscard]] int onlineCount() const;
    [[nodiscard]] int totalUnreadCount() const;

    bool restore(const Network::PeerEndpoint &endpoint, const QString &lastMessage, const QString &lastTime, int unreadCount);
    bool upsert(const Network::PeerEndpoint &endpoint, bool *inserted = nullptr);
    bool setOffline(const QString &peerId);
    void updateConversation(const QString &peerId, const QString &message, const QString &time, bool incrementUnread);
    void clearUnread(const QString &peerId);

signals:
    void unreadCountChanged();

private:
    struct PeerItem
    {
        Network::PeerEndpoint endpoint;
        QString initial;
        QString avatarColor;
        QString lastMessage;
        QString lastTime;
        bool online = false;
        int unread = 0;
    };

    [[nodiscard]] QVariantMap peerInfoAt(int row) const;
    [[nodiscard]] static QString initialForName(const QString &name);
    [[nodiscard]] static QString colorForId(const QString &peerId);

    QList<PeerItem> m_peers;
};

#endif // PEERLISTMODEL_H
