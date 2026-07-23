/**
 * @file peerlistmodel.h
 * @brief 声明供 QML 使用的已发现节点列表模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef PEERLISTMODEL_H
#define PEERLISTMODEL_H

#include "core/networktypes.h"

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

    /**
     * @brief 构造节点列表模型。
     * @param parent 可选的 QObject 父对象。
     */
    explicit PeerListModel(QObject *parent = nullptr);

    /**
     * @brief 返回节点数量。
     * @param parent 父模型索引；列表模型应传入无效索引。
     * @return 节点数量。
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief 返回指定节点的角色数据。
     * @param index 节点模型索引。
     * @param role 待读取的模型角色。
     * @return 对应角色的数据；索引或角色无效时返回空值。
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    /**
     * @brief 返回 QML 可访问的模型角色名称。
     * @return 角色编号到角色名称的映射。
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief 查找节点在列表中的索引。
     * @param peerId 待查找的节点标识。
     * @return 节点索引；节点未知时返回 @c -1。
     */
    [[nodiscard]] int indexOf(const QString &peerId) const;
    /**
     * @brief 返回指定节点的 QML 属性映射。
     * @param peerId 待查询的节点标识。
     * @return 节点属性；节点未知时返回空映射。
     */
    [[nodiscard]] QVariantMap peerInfo(const QString &peerId) const;
    /**
     * @brief 返回当前在线的节点数量。
     * @return 在线节点数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有节点的未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 替换模型中的全部节点。
     * @param items 新的节点列表。
     */
    void setItems(QList<Item> items);

signals:
    /** @brief 未读消息总数发生变化时发出。 */
    void unreadCountChanged();

private:
    QList<Item> m_peers;
};

#endif // PEERLISTMODEL_H
