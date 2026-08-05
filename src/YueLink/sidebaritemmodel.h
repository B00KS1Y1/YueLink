/**
 * @file sidebaritemmodel.h
 * @brief 声明联系人页与统一会话页共用的侧栏列表模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-06
 */

#ifndef SIDEBARITEMMODEL_H
#define SIDEBARITEMMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>

class SidebarItemModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        ItemIdRole = Qt::UserRole + 1,
        ItemKindRole,
        TitleRole,
        InitialRole,
        StatusTextRole,
        LastMessageRole,
        LastTimeRole,
        AvatarColorRole,
        OnlineRole,
        UnreadRole,
        PeerIdRole,
        MemberCountRole,
        OnlineCountRole,
        SortTimestampRole
    };

    struct Item
    {
        QString itemId;
        QString itemKind;
        QString title;
        QString initial;
        QString statusText;
        QString lastMessage;
        QString lastTime;
        QString avatarColor;
        QString peerId;
        int unread = 0;
        int memberCount = 0;
        int onlineCount = 0;
        qint64 sortTimestamp = 0;
        bool online = false;
    };

    /**
     * @brief 构造侧栏列表模型。
     * @param parent 可选的 QObject 父对象。
     */
    explicit SidebarItemModel(QObject *parent = nullptr);
    /**
     * @brief 返回列表行数。
     * @param parent 父索引；列表模型应传入无效索引。
     * @return 行数。
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief 返回模型角色数据。
     * @param index 行索引。
     * @param role 角色编号。
     * @return 角色数据；索引或角色无效时为空。
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    /**
     * @brief 返回 QML 角色名称。
     * @return 角色编号到名称的映射。
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    /**
     * @brief 根据条目标识返回属性映射。
     * @param itemId 条目标识。
     * @return 条目属性；不存在时返回空映射。
     */
    [[nodiscard]] QVariantMap itemInfo(const QString &itemId) const;
    /**
     * @brief 返回在线条目数量。
     * @return online 为真的条目数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有条目未读数之和。
     * @return 未读总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 替换全部条目。
     * @param items 新条目列表。
     */
    void setItems(QList<Item> items);

signals:
    /** @brief 未读总数发生变化时发出。 */
    void unreadCountChanged();

private:
    QList<Item> m_items;
};

#endif // SIDEBARITEMMODEL_H
