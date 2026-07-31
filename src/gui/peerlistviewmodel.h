/**
 * @file peerlistviewmodel.h
 * @brief 声明好友列表的 QML 视图模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef PEERLISTVIEWMODEL_H
#define PEERLISTVIEWMODEL_H

#include "peerlistmodel.h"

#include <QDateTime>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QVariantMap>

class ChatCoordinator;

class PeerListViewModel final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造好友列表视图模型。
     * @param coordinator 非拥有的聊天协调器；必须覆盖本对象生命周期。
     * @param parent 可选的 QObject 父对象。
     */
    explicit PeerListViewModel(ChatCoordinator *coordinator,
                               QObject *parent = nullptr);
    /** @brief 销毁好友列表视图模型。 */
    ~PeerListViewModel() override;

    /**
     * @brief 返回供 QML 使用的筛选模型。
     * @return 好友列表筛选模型指针。
     */
    [[nodiscard]] QAbstractItemModel *model();
    /**
     * @brief 返回当前好友搜索文本。
     * @return 未经裁剪的搜索文本。
     */
    [[nodiscard]] QString searchText() const;
    /**
     * @brief 更新好友搜索文本。
     * @param text 新搜索文本；匹配时忽略大小写和首尾空白。
     */
    void setSearchText(const QString &text);
    /**
     * @brief 返回当前在线好友数量。
     * @return 在线好友数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有好友未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 返回指定好友的 QML 属性映射。
     * @param peerId 好友节点标识。
     * @return 好友属性；好友未知时返回空映射。
     */
    [[nodiscard]] QVariantMap peerInfo(const QString &peerId) const;

signals:
    /** @brief 好友搜索文本发生变化时发出。 */
    void searchTextChanged();
    /** @brief 在线好友数量发生变化时发出。 */
    void onlineCountChanged();
    /** @brief 未读消息总数发生变化时发出。 */
    void totalUnreadCountChanged();
    /**
     * @brief 发现此前未知的好友时发出。
     * @param peerId 新好友节点标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 已知好友状态发生变化时发出。
     * @param peerId 已更新好友节点标识。
     */
    void peerUpdated(const QString &peerId);

private:
    /** @brief 将协调器中的好友状态同步到列表模型。 */
    void synchronize();
    /**
     * @brief 返回名称首字符的大写形式。
     * @param name 待处理的显示名称。
     * @return 名称首字符；名称为空时返回占位符。
     */
    [[nodiscard]] static QString initialForName(const QString &name);
    /**
     * @brief 将时间戳格式化为好友列表显示文本。
     * @param timestamp 待格式化的时间戳。
     * @return 本地化的简短时间文本。
     */
    [[nodiscard]] static QString displayTime(const QDateTime &timestamp);
    /**
     * @brief 为节点标识生成稳定头像颜色。
     * @param peerId 节点标识。
     * @return 颜色表中的十六进制颜色字符串。
     */
    [[nodiscard]] static QString colorForId(const QString &peerId);

    ChatCoordinator *m_coordinator = nullptr;
    PeerListModel m_model;
    QSortFilterProxyModel m_filterModel;
    QString m_searchText;
};

#endif // PEERLISTVIEWMODEL_H
