/**
 * @file peerlistviewmodel.h
 * @brief 声明联系人页筛选与侧栏模型视图模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef PEERLISTVIEWMODEL_H
#define PEERLISTVIEWMODEL_H

#include "YueLink/sidebaritemmodel.h"

#include <QObject>
#include <QSortFilterProxyModel>

class ChatCoordinator;

class PeerListViewModel final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造联系人视图模型。
     * @param coordinator 非拥有的聊天协调器。
     * @param parent 可选的 QObject 父对象。
     */
    explicit PeerListViewModel(ChatCoordinator *coordinator,
                               QObject *parent = nullptr);
    /** @brief 销毁联系人视图模型。 */
    ~PeerListViewModel() override;
    /**
     * @brief 返回供 QML 使用的筛选模型。
     * @return 筛选模型指针。
     */
    [[nodiscard]] QAbstractItemModel *model();
    /**
     * @brief 返回搜索文本。
     * @return 当前搜索文本。
     */
    [[nodiscard]] QString searchText() const;
    /**
     * @brief 更新搜索文本。
     * @param text 新搜索文本。
     */
    void setSearchText(const QString &text);
    /**
     * @brief 返回在线联系人数。
     * @return 在线联系人数。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回联系人属性映射。
     * @param peerId 联系人设备标识。
     * @return 联系人属性；不存在时为空。
     */
    [[nodiscard]] QVariantMap peerInfo(const QString &peerId) const;

signals:
    /** @brief 搜索文本发生变化时发出。 */
    void searchTextChanged();
    /** @brief 在线联系人数发生变化时发出。 */
    void onlineCountChanged();
    /**
     * @brief 发现联系人时发出。
     * @param peerId 联系人设备标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 联系人更新时发出。
     * @param peerId 联系人设备标识。
     */
    void peerUpdated(const QString &peerId);

private:
    /** @brief 从协调器同步全部联系人。 */
    void synchronize();
    /**
     * @brief 返回显示名称首字符。
     * @param name 显示名称。
     * @return 大写首字符或问号。
     */
    [[nodiscard]] static QString initialForName(const QString &name);
    /**
     * @brief 为标识生成稳定头像颜色。
     * @param id 稳定标识。
     * @return 十六进制颜色。
     */
    [[nodiscard]] static QString colorForId(const QString &id);

    ChatCoordinator *m_coordinator = nullptr;
    SidebarItemModel m_model;
    QSortFilterProxyModel m_filterModel;
    QString m_searchText;
};

#endif // PEERLISTVIEWMODEL_H
